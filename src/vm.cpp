
#include "vm.hpp"
#include "debug.hpp"

#include <stdio.h>


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
        disasm.disassembleInstruction(chunk_, (int)(ip_ - chunk_->getCode()));
#endif

        uint8_t instr = readByte_();
        switch( instr ){
            case OpCode::RETURN:{
                return InterpretResult::OK;
            }
            case OpCode::CONSTANT:{
                Value constant = chunk_->getConstant(readByte_());
                printValue(constant);
                printf("\n");
            }
        }
    }
}
