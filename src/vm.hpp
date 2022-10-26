#pragma once

#include "chunk.hpp"
#include "value.hpp"
#include "object.hpp"
#include "str.hpp"

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
    Value peek(int index);  // index counts from top (end) of stack

    // adding/removing objects, called from Obj(), ~Obj()
    void registerObj(Obj * obj);
    void deregisterObj(Obj * obj);

    // intern string
    ObjString * addString(char const * str, int len);
    ObjString * addString(std::string str);

private:
    InterpretResult run_();
    inline uint8_t readByte_() { return *ip_++; }
    inline void resetStack_() { stackTop_ = stack_; }
    bool binaryOp_(uint8_t op);
    bool isTruthy_(Value value);
    void concatenate_();
    void runtimeError_(const char* format, ...);
    void freeObjects_();

    static int const STACK_MAX = 256;

    Chunk * chunk_;     // current chunk of bytecode
    uint8_t * ip_;      // instruction pointer
    Value stack_[STACK_MAX];
    Value * stackTop_;  // points past the last value in the stack
    Obj * objects_;     // linked list of objects
    InternedStringSet internedStrings_;
};
