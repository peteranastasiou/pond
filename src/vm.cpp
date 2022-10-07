
#include "vm.hpp"
#include "debug.hpp"
#include "compiler.hpp"

#include <stdio.h>
#include <stdlib.h>


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

void Vm::binaryOp_(uint8_t op){
    Value b = pop();
    Value a = pop();
    switch( op ){
        case OpCode::ADD:
            push(a + b);
            break;
        case OpCode::SUBTRACT:
            push(a - b);
            break;
        case OpCode::MULTIPLY:
            push(a * b);
            break;
        case OpCode::DIVIDE:
            push(a / b);
            break;
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
            case OpCode::ADD:
            case OpCode::SUBTRACT:
            case OpCode::MULTIPLY:
            case OpCode::DIVIDE:{
                binaryOp_(instr);
                break;
            }
            case OpCode::NEGATE:{
                push(-pop());
                break;
            }
            case OpCode::RETURN:{
                printValue(pop());
                printf("\n");
                return InterpretResult::OK;
            }
            case OpCode::CONSTANT:{
                Value constant = chunk_->getConstant(readByte_());
                push(constant);
                break;
            }
            default:{
                printf("Fatal error: unknown opcode %d\n", (int)instr);
                exit(1);
            }
        }
    }
}
