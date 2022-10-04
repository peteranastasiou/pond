
#include "debug.hpp"

#include <stdio.h>

Dissassembler::Dissassembler(){
}

Dissassembler::~Dissassembler(){
}

void Dissassembler::disassembleChunk(Chunk * chunk, char const * name){
  printf("== %s ==\n", name);

  for (int offset = 0; offset < chunk->count();) {
    offset = disassembleInstruction(chunk, offset);
  }
}

int Dissassembler::disassembleInstruction(Chunk * chunk, int offset){
  printf("%04i ", offset);

  uint8_t instr = chunk->code[(size_t)offset];
  switch(instr){
    case OpCode::CONSTANT:
      return constantInstruction_("CONSTANT", chunk, offset);

    case OpCode::RETURN:
      return simpleInstruction_("RETURN", offset);

    default:
      printf("Unknown opcode %i\n", instr);
      return offset + 1;
  }
}

int Dissassembler::constantInstruction_(char const * name, Chunk * chunk, int offset){
  uint8_t constantIdx = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constantIdx);
  printValue(chunk->constants[constantIdx]);
  printf("'\n");
  return offset + 2;
}

int Dissassembler::simpleInstruction_(char const * name, int offset){
  printf("%s\n", name);
  return offset + 1;
}
