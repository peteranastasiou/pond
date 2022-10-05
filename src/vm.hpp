#pragma once

#include "chunk.hpp"

enum class InterpretResult {
    OK,
    COMPILE_ERR,
    RUNTIME_ERR
};

class Vm {
public:
    Vm();
    
    ~Vm();

    InterpretResult interpret(Chunk * chunk);

private:
    InterpretResult run_();
    inline uint8_t readByte_() { return *ip_++; }

    Chunk * chunk_;     // current chunk of bytecode
    uint8_t * ip_;      // instruction pointer
};
