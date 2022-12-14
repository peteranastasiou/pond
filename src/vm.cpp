
#include "vm.hpp"
#include "debug.hpp"
#include "compiler.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


Vm::Vm() {
    objects_ = nullptr;
    resetStack_();
}

Vm::~Vm() {
    freeObjects_();
}

InterpretResult Vm::interpret(char const * source) {
    Compiler compiler(this);
    Chunk chunk;
    if( !compiler.compile(source, chunk) ){
        return InterpretResult::COMPILE_ERR;
    }
    chunk_= &chunk;
    ip_ = chunk_->getCode();
    return run_();
}

void Vm::registerObj(Obj * obj){
    obj->next = objects_;  // previous head
    objects_ = obj;        // new head
}

void Vm::deregisterObj(Obj * obj){
    // TODO
}

void Vm::push(Value value) {
    *stackTop_ = value;
    stackTop_++;
}

Value Vm::pop() {
    stackTop_--;
    if( stackTop_ == stack_-1 ){
        printf("Fatal: pop empty stack\n");
        exit(1);
    }
    return *stackTop_;
}

Value Vm::peek(int index) {
    return stackTop_[-1 - index];
}

bool Vm::binaryOp_(uint8_t op){
    if( !peek(0).isNumber() || !peek(1).isNumber() ){
        runtimeError_("Operands must be numbers.");
        return false;
    }

    double b = pop().as.number;
    double a = pop().as.number;
    switch( op ){
        case OpCode::GREATER:       push(Value::boolean( a > b )); break;
        case OpCode::GREATER_EQUAL: push(Value::boolean( a >= b )); break;
        case OpCode::LESS:          push(Value::boolean( a < b )); break;
        case OpCode::LESS_EQUAL:    push(Value::boolean( a <= b )); break;
        case OpCode::SUBTRACT:      push(Value::number( a - b )); break;
        case OpCode::MULTIPLY:      push(Value::number( a * b )); break;
        case OpCode::DIVIDE:        push(Value::number( a / b )); break;
    }
    return true;
}

bool Vm::isTruthy_(Value value) {
    switch( value.type ){
        case Value::NIL:  return false;
        case Value::BOOL: return value.as.boolean;
        default:          return true;  // All other types are true!
    }
}

void Vm::concatenate_() {
    Value bValue = pop();
    ObjString * b = bValue.toString(this);
    ObjString * a = pop().asObjString();
    push( Value::object(ObjString::concatenate(this, a, b)) );
}

Value Vm::readConstant_() {
    // look up constant from bytecode reference
    return chunk_->getConstant(readByte_());
}

ObjString * Vm::readString_() {
    // look up constant from bytecode and cast to string:
    Value constant = readConstant_();
    assert(constant.isString());
    return constant.asObjString();
}

InterpretResult Vm::run_() {
#ifdef DEBUG_TRACE_EXECUTION
    Dissassembler disasm;  

    internedStrings_.debug();
    debugObjectLinkedList(objects_);

    printf("Constants:\n");
    for( uint8_t i =0; i < chunk_->numConstants(); ++i ){
        printf(" %i ", i);
        Value v = chunk_->getConstant(i);
        if( v.isObject() ) printf("%p [", v.as.obj);
        v.print();
        printf("]\n");
    }
    printf("Globals:\n");
    globals_.debug();
    printf("====\n");

#endif

    for(;;) {

#ifdef DEBUG_TRACE_EXECUTION
        printf("          stack: ");
        for( Value * slot = stack_; slot < stackTop_; slot++ ){
            printf("[ ");
            slot->print();
            printf(" ]");
        }
        printf("\n");

        disasm.disassembleInstruction(chunk_, (int)(ip_ - chunk_->getCode()));
#endif

        uint8_t instr = readByte_();
        switch( instr ){
            case OpCode::CONSTANT:{
                push(readConstant_());
                break;
            }
            case OpCode::NIL: push(Value::nil()); break;
            case OpCode::TRUE: push(Value::boolean(true)); break;
            case OpCode::FALSE: push(Value::boolean(false)); break;
            case OpCode::POP: pop(); break;
            case OpCode::DEFINE_GLOBAL: {
                // NOTE: re-defining globals is allowed!
                ObjString * name = readString_();
                globals_.set(name, peek(0));
                pop(); // Note: lox has this late pop as `set` might trigger garbage collection
                break;
            }
            case OpCode::GET_GLOBAL: {
                ObjString * name = readString_();
                Value value;
                if( !globals_.get(name, value) ){
                    runtimeError_("Undefined variable '%s'.", name->get());
                    return InterpretResult::RUNTIME_ERR;
                }
                push(value);
                break;
            }
            case OpCode::SET_GLOBAL: {
                ObjString * name = readString_();
                if( globals_.set(name, peek(0)) ){
                    // Didn't expect this to be a new variable!
                    globals_.remove(name); // undo the operation
                    runtimeError_("Undefined variable '%s'.", name->get());
                    return InterpretResult::RUNTIME_ERR;
                }
                // don't pop: the assignment can be used in an expression
                break;
            }
            case OpCode::EQUAL: {
                push(Value::boolean( pop().equals(pop()) ));
                break;
            }
            case OpCode::NOT_EQUAL: {
                push(Value::boolean( !pop().equals(pop()) ));
                break;
            }
            case OpCode::GREATER:
            case OpCode::GREATER_EQUAL:
            case OpCode::LESS:
            case OpCode::LESS_EQUAL:
            case OpCode::SUBTRACT:
            case OpCode::MULTIPLY:
            case OpCode::DIVIDE:{
                if( !binaryOp_(instr) ) return InterpretResult::RUNTIME_ERR;
                break;
            }
            case OpCode::ADD:{
                if( peek(1).isString() ){ 
                    // implicitly convert second operand to string
                    concatenate_();

                }else if( peek(0).isNumber() && peek(1).isNumber() ){
                    double b = pop().as.number;
                    double a = pop().as.number;
                    push(Value::number( a + b ));
                }else{
                    runtimeError_("Invalid operands for +");
                    return InterpretResult::RUNTIME_ERR;
                }
                break;
            }
            case OpCode::NEGATE:{
                // ensure is numeric:
                if( !peek(0).isNumber() ){
                    runtimeError_("Operand must be a number");
                    return InterpretResult::RUNTIME_ERR;
                }

                push( Value::number(-pop().as.number) );
                break;
            }
            case OpCode::NOT:{
                push(Value::boolean(!isTruthy_(pop())));
                break;
            }
            case OpCode::PRINT:{
                pop().print();
                printf("\n");
                break;
            }
            case OpCode::RETURN:{
                return InterpretResult::OK;
            }
            default:{
                printf("Fatal error: unknown opcode %d\n", (int)instr);
                exit(1);
            }
        }
    }
}

void Vm::runtimeError_(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    int offset = (int)(ip_ - chunk_->getCode() - 1);
    int line = chunk_->getLineNumber(offset);
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack_();
}

void Vm::freeObjects_() {
    // iterate linked list of objects, deleting them
    Obj * obj = objects_;
    while( obj != nullptr ){
        Obj * next  = obj->next;
        delete obj;
        obj = next;
    }
}
