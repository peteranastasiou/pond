
#include "vm.hpp"
#include "debug.hpp"
#include "compiler.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


Vm::Vm() {
    resetStack_();
}

Vm::~Vm() {
}

InterpretResult Vm::interpret(char const * source) {
    Compiler compiler;
    Chunk chunk;
    if( !compiler.compile(source, chunk) ){
        return InterpretResult::COMPILE_ERR;
    }
    chunk_= &chunk;
    ip_ = chunk_->getCode();
    return run_();
}

void Vm::push(Value value) {
    *stackTop_ = value;
    stackTop_++;
}

Value Vm::pop() {
    stackTop_--;
    if( stackTop_ == stack_-1 ){
        printf("Fatal: pop empty stack\n");
        exit(1);  // todo error handling
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
    double c;
    switch( op ){
        case OpCode::ADD:       c = a + b; break;
        case OpCode::SUBTRACT:  c = a - b; break;
        case OpCode::MULTIPLY:  c = a * b; break;
        case OpCode::DIVIDE:    c = a / b; break;
    }
    push(Value::number(c));
    return true;
}

bool Vm::isTruthy_(Value value) {
    switch( value.type ){
        case Value::NIL:  return false;
        case Value::BOOL: return value.as.boolean;
        default:          return true;  // All other types are true!
    }
}

InterpretResult Vm::run_() {
#ifdef DEBUG_TRACE_EXECUTION
      Dissassembler disasm;  
#endif

    for(;;) {

#ifdef DEBUG_TRACE_EXECUTION
        printf("stack: ");
        for( Value * slot = stack_; slot < stackTop_; slot++ ){
            printf("[ ");
            printValue(*slot);
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
            case OpCode::ADD:
            case OpCode::SUBTRACT:
            case OpCode::MULTIPLY:
            case OpCode::DIVIDE:{
                if( !binaryOp_(instr) ) return InterpretResult::RUNTIME_ERR;
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
            }
            case OpCode::RETURN:{
                printValue(pop());
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
