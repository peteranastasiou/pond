
#include "chunk.hpp"
#include "debug.hpp"


int main(int argc, char const * argv[]) {
    Chunk chunk;

    uint8_t constantIdx = chunk.addConstant(1.2);
    chunk.write(OpCode::CONSTANT);
    chunk.write(constantIdx);
    chunk.write(OpCode::RETURN);

    Dissassembler ds;
    ds.disassembleChunk(&chunk, "test chunk");
    return 0;
}
