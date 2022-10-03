
#include "chunk.hpp"

#include <stdlib.h>  // exit


static int const MAX_COUNT_ = 65535;

Chunk::Chunk() {
}

Chunk::~Chunk() {
}

void Chunk::write(uint8_t byte) {
    if( code.size() > MAX_COUNT_ ){
        // TODO fatal error
        exit(1);
    }
    code.push_back(byte);
}

int Chunk::count() {
    return (int)code.size();
}
