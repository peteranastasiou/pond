
#include "vm.hpp"
#include "debug.hpp"

#include <stdio.h>
#include <stdlib.h>


Vm::Vm() {
    resetStack_();
}

Vm::~Vm() {
}

InterpretResult Vm::interpret(Chunk * chunk) {
    chunk_= chunk;
    ip_ = chunk_->getCode();
    return run_();
}

void Vm::push(Value value) {
    *stackTop_ = value;
    stackTop_++;
}

Value Vm::pop() {
    stackTop_--;
    return *stackTop_;
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
