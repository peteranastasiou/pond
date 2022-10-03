#pragma once

#include <stdint.h>
#include <stddef.h>
#include <vector>

namespace OpCode {
enum {
    RETURN,
};
}

class Chunk {
public:
    Chunk();
    ~Chunk();
    void write(uint8_t byte);
    int count();

private:
    std::vector<uint8_t> code;

    // Disassembler needs access within the chunk:
    friend int disassembleInstruction(Chunk * chunk, int offset);
};

