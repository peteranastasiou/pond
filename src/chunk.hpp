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

class Chunk {
public:
    Chunk();
    ~Chunk();
    void write(uint8_t byte, int line);
    uint8_t addConstant(Value value);
    int count();

private:
    std::vector<uint8_t> code;
    std::vector<int> lines;         // line numbers corresponding to bytecode array
    std::vector<Value> constants;

    // Disassembler needs access within the chunk:
    friend class Dissassembler;
};

