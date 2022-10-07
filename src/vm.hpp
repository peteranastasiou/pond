#pragma once

#include "chunk.hpp"
#include "value.hpp"

enum class InterpretResult {
    OK,
    COMPILE_ERR,
    RUNTIME_ERR
};

class Vm {
public:
    Vm();
    
    ~Vm();

    InterpretResult interpret(char const * source);

    // stack operations:
    void push(Value value);
    Value pop();

private:
    InterpretResult run_();
    inline uint8_t readByte_() { return *ip_++; }
    inline void resetStack_() { stackTop_ = stack_; }
    void binaryOp_(uint8_t op);

    static int const STACK_MAX = 256;

    Chunk * chunk_;     // current chunk of bytecode
    uint8_t * ip_;      // instruction pointer
    Value stack_[STACK_MAX];
    Value * stackTop_;  // points past the last value in the stack
};
