
#include "chunk.hpp"
#include "debug.hpp"


int main(int argc, char const * argv[]) {
    Chunk chunk;
    chunk.write(OpCode::RETURN);
    disassembleChunk(&chunk, "test chunk");
    return 0;
}
