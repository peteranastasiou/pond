
#include "vm.hpp"
#include "debug.hpp"
#include "compiler.hpp"

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
    printf("Deregister obj\n");
}

ObjString * Vm::addString(char const * str, int len){
    // Elevate to std::string
    std::string s(str, str+len);
    return addString(s);
}

ObjString * Vm::addString(std::string str){
    {
        // Search if string is already interned:
        Lookup lookup{str};
        auto search = internedStrings_.find(&lookup);
        if( search != internedStrings_.end() ){
            // Found it
            Key * key = *search;
            ObjString *ostr = (ObjString *)key;

            printf("String [%s] already exists at %p: [%s]\n", str.c_str(), key, ostr->get().c_str());
            // Must be an ObjString because thats all we ever add to the set
            return (ObjString *)key;
        }
    }
    // Not in the set - create a new memory managed object:
    ObjString * ostr = new ObjString(this, str);
    internedStrings_.emplace(ostr);
    debugInternedStringSet(internedStrings_);
    printf("New string [%s] at %p: [%s]\n", str.c_str(), ostr, ostr->get().c_str());
    return ostr;
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
    std::string b = bValue.toString();
    std::string a = pop().asString();
    push( Value::object(addString(a + b)) );
}

InterpretResult Vm::run_() {
#ifdef DEBUG_TRACE_EXECUTION
      Dissassembler disasm;  
#endif

    debugInternedStringSet(internedStrings_);
    debugObjectLinkedList(objects_);

    printf("Constants:\n");
    for( uint8_t i =0; i < chunk_->numConstants(); ++i ){
        printf(" %i ", i);
        Value v = chunk_->getConstant(i);
        if( v.isObject() ) printf("%p [", v.as.obj);
        v.print();
        printf("]\n");
    }

    for(;;) {

#ifdef DEBUG_TRACE_EXECUTION
        printf("stack: ");
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
                Value constant = chunk_->getConstant(readByte_());
                push(constant);
                break;
            }
            case OpCode::NIL: push(Value::nil()); break;
            case OpCode::TRUE: push(Value::boolean(true)); break;
            case OpCode::FALSE: push(Value::boolean(false)); break;
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
            case OpCode::RETURN:{
                pop().print();
                printf("\n");
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

    size_t instruction = ip_ - chunk_->getCode() - 1;
    int line = 0;  // TODO decypher line number!
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
