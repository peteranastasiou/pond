#pragma once

#include "value.hpp"

#include <stdint.h>
#include <stddef.h>
#include <vector>

namespace OpCode {
enum {
    CONSTANT,       // Load a constant (literal) from the chunk
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

    // Get the length of the bytecode array
    int count();

    // Get a pointer to the bytecode array
    uint8_t * getCode();

    // Add a constant value and return its index
    uint8_t addConstant(Value value);

    // Get a constant value by its index
    Value getConstant(uint8_t index);

private:
    std::vector<uint8_t> code;
    std::vector<LineNum> lines;         // line numbers corresponding to bytecode array
    std::vector<Value> constants;

    // Disassembler needs access within the chunk:
    friend class Dissassembler;
};

