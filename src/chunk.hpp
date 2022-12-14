#pragma once

#include "value.hpp"

#include <stdint.h>
#include <stddef.h>
#include <vector>

namespace OpCode {
enum {
    // Literals:
    CONSTANT,       // Push a constant (literal) from the chunk
    NIL,            // Push nil to the stack
    TRUE,           // Push true to the stack
    FALSE,          // Push false to the stack
    POP,            // Pop 1 value from the stack
    DEFINE_GLOBAL,  // Define a global variable
    GET_GLOBAL,     // Push the value of a global to the stack
    SET_GLOBAL,     // Set the value of a variable
    // Binary operators: take two values from the stack and push one:
    EQUAL,
    NOT_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    // Unary operators: take one value, push one value:
    NEGATE,
    NOT,
    // Control flow:
    PRINT,
    RETURN,
};
}

struct LineNum {
    uint16_t line;  // line number
    uint8_t count;  // number of instructions on the line
};

class Chunk {
public:
    Chunk();

    ~Chunk();

    // append to bytecode array
    void write(uint8_t byte, uint16_t line);
    
    // Get a line number corresponding to position in bytecode array
    uint16_t getLineNumber(int offset);

    // Get the length of the bytecode array
    int count();

    // Get a pointer to the bytecode array
    uint8_t * getCode();

    // Add a constant value and return its index
    uint8_t addConstant(Value value);

    // Get a constant value by its index
    Value getConstant(uint8_t index);

    uint8_t numConstants();

    static uint8_t const MAX_CONSTANTS = 255;  // constant index must fit in a byte (for now)

private:
    std::vector<uint8_t> code;
    std::vector<uint16_t> lines;    // line numbers corresponding to bytecode array
    std::vector<Value> constants;

    // Disassembler needs access within the chunk:
    friend class Dissassembler;
};

