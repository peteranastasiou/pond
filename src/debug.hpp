#pragma once

#include "chunk.hpp"

#include <stdint.h>
#include <stddef.h>


class Dissassembler {
public:
    Dissassembler();
    ~Dissassembler();

    void disassembleChunk(Chunk * chunk, char const * name);
    int disassembleInstruction(Chunk * chunk, int offset);

private:
    int constantInstruction_(char const * name, Chunk * chunk, int offset);
    int simpleInstruction_(char const * name, int offset);
};
