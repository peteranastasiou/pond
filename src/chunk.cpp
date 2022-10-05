
#include "chunk.hpp"

#include <stdlib.h>  // exit


static int const MAX_COUNT_ = 65535;
static int const MAX_CONSTANTS_ = 256;  // constant index must fit in a byte (for now)

Chunk::Chunk() {
}

Chunk::~Chunk() {
}

void Chunk::write(uint8_t byte, int line) {
    if( code.size() >= MAX_COUNT_ ){
        // TODO fatal error
        exit(1);
    }
    code.push_back(byte);
    lines.push_back(line);   // TODO run-length encoding
}

uint8_t Chunk::addConstant(Value value) {
    int size = (int)constants.size();
    if( size >= MAX_CONSTANTS_ ){
        // TODO fatal error
        exit(1);
    }
    constants.push_back(value);
    return (uint8_t)size; // index of new constant
}

int Chunk::count() {
    return (int)code.size();
}
