#include <stdio.h>

#include "chunk.h"
#include "debug.h"

int main() {
    Chunk chunk;
    initChunk(&chunk);
    int constant = addConstant(&chunk, 2.79);
    writeChunk(&chunk, OP_CONSTANT);
    writeChunk(&chunk, constant);
    writeChunk(&chunk, OP_RETURN);
    disassembleChunk(&chunk, "test chunk");
    freeChunk(&chunk);
    return 0;
}