
#include "vm.hpp"

#include <stdio.h>


Vm::Vm() {

}

Vm::~Vm() {

}

InterpretResult Vm::interpret(Chunk * chunk) {
    chunk_= chunk;
    ip_ = chunk_->getCode();
    return run_();
}

InterpretResult Vm::run_() {
    for(;;) {
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
