
#include "vm.hpp"
#include "chunk.hpp"
#include "debug.hpp"

#include <stdio.h>


int main(int argc, char const * argv[]) {
    Vm vm;
    Chunk chunk;

    chunk.write(OpCode::CONSTANT, 123);
    chunk.write(chunk.addConstant(1.2), 123);
    chunk.write(OpCode::CONSTANT, 123);
    chunk.write(chunk.addConstant(3.4), 123);
    chunk.write(OpCode::ADD, 123);
    chunk.write(OpCode::CONSTANT, 123);
    chunk.write(chunk.addConstant(5.6), 123);
    chunk.write(OpCode::DIVIDE, 123);
    chunk.write(OpCode::NEGATE, 123);
    chunk.write(OpCode::RETURN, 124);

    // Dissassembler ds;
    // ds.disassembleChunk(&chunk, "test chunk");

    vm.interpret(&chunk);

    return 0;
}
