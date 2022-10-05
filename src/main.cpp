
#include "chunk.hpp"
#include "debug.hpp"


int main(int argc, char const * argv[]) {
    Chunk chunk;

    uint8_t constantIdx = chunk.addConstant(1.2);
    chunk.write(OpCode::CONSTANT, 123);
    chunk.write(constantIdx, 123);
    chunk.write(OpCode::RETURN, 124);

    Dissassembler ds;
    ds.disassembleChunk(&chunk, "test chunk");
    return 0;
}
