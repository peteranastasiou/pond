
#include "debug.hpp"

#include <stdio.h>

static int simpleInstruction_(char const * name, int offset);

void disassembleChunk(Chunk * chunk, char const * name){
  printf("== %s ==\n", name);

  for (int offset = 0; offset < chunk->count();) {
    offset = disassembleInstruction(chunk, offset);
  }
}

int disassembleInstruction(Chunk * chunk, int offset){
    printf("%04i ", offset);

    uint8_t instr = chunk->code[(size_t)offset];
    switch(instr){
        case OpCode::RETURN:
            return simpleInstruction_("RETURN", offset);
        default:
            printf("Unknown opcode %i\n", instr);
            return offset + 1;
    }
}

static int simpleInstruction_(char const * name, int offset){
    printf("%s\n", name);
    return offset + 1;
}
