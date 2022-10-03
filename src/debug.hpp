#pragma once

#include "chunk.hpp"

#include <stdint.h>
#include <stddef.h>


void disassembleChunk(Chunk * chunk, char const * name);
int disassembleInstruction(Chunk * chunk, int offset);

