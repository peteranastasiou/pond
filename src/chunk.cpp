
#include "chunk.hpp"

#include <stdlib.h>  // exit


static int const MAX_COUNT_ = 65535;

Chunk::Chunk() {
}

Chunk::~Chunk() {
}

void Chunk::write(uint8_t byte, uint16_t line) {
    if( code.size() >= MAX_COUNT_ ){
        // TODO fatal error
        exit(1);
    }
    code.push_back(byte);
    
    if( lines.size() == 0
        || lines.back().line != line
        || lines.back().count == 0xFF ){    // TODO test > 256 bytecode bytes on 1 line
        // new line
        lines.push_back({line, 1});
    }else{
        // same line
        lines.back().count ++;
    }
}

int Chunk::count() {
    return (int)code.size();
}

uint8_t * Chunk::getCode() {
    return &code[0];
}

uint8_t Chunk::addConstant(Value value) {
    int size = (int)constants.size();
    if( size < MAX_CONSTANTS ){
        constants.push_back(value);
        return (uint8_t)size; // index of new constant
    }else{
        return MAX_CONSTANTS; // full!
    }
}

Value Chunk::getConstant(uint8_t index) {
    return constants[index];
}

uint8_t Chunk::numConstants() {
    return (uint8_t)constants.size();
}
