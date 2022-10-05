
#include "debug.hpp"

#include <stdio.h>
#include <stdlib.h>


Dissassembler::Dissassembler(){
}

Dissassembler::~Dissassembler(){
}

void Dissassembler::disassembleChunk(Chunk * chunk, char const * name){
  printf("== %s ==\n", name);

  // first line number
  int lineIdx = 0;
  int lineInstrCount = chunk->lines[lineIdx].count; // number of bytecode bytes per line

  for (int offset = 0; offset < chunk->count();) {
    int incr = disassembleInstruction_(chunk, offset, chunk->lines[lineIdx].line);
    offset += incr;
    if( lineInstrCount -= incr <= 0 ){
      // new line:
      lineIdx ++;
      lineInstrCount = chunk->lines[lineIdx].count;
    }
  }
}

int Dissassembler::disassembleInstruction(Chunk * chunk, int offset){
  // TODO decypher the line number
  exit(0);
}

int Dissassembler::disassembleInstruction_(Chunk * chunk, int offset, int line){
  printf("%04i ", offset);
  printf("%4d ", line);

  uint8_t instr = chunk->code[(size_t)offset];
  switch(instr){
    case OpCode::CONSTANT:
      return constantInstruction_("CONSTANT", chunk, offset);

    case OpCode::RETURN:
      return simpleInstruction_("RETURN", offset);

    default:
      printf("Unknown opcode %i\n", instr);
      return 1;
  }
}

int Dissassembler::constantInstruction_(char const * name, Chunk * chunk, int offset){
  uint8_t constantIdx = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constantIdx);
  printValue(chunk->constants[constantIdx]);
  printf("'\n");
  return 2;
}

int Dissassembler::simpleInstruction_(char const * name, int offset){
  printf("%s\n", name);
  return 1;
}
