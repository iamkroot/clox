//
// Created by kroot on 15/7/19.
//

#include <stdlib.h>

#include "chunk.h"
#include "memory.h"
#include "value.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lineCount = 0;
    chunk->lineCapacity = 0;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(LineInfo, chunk->lines, chunk->lineCapacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    if (chunk->count >= chunk->capacity) {
        int oldCap = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCap);
        chunk->code = GROW_ARRAY(chunk->code, uint8_t, oldCap, chunk->capacity);
    }
    chunk->code[chunk->count] = byte;
    chunk->count++;
    if (chunk->lineCount > 0 && chunk->lines[chunk->lineCount - 1].line == line) {
        return;
    }
    if (chunk->lineCount >= chunk->lineCapacity) {
        int oldCap = chunk->lineCapacity;
        chunk->lineCapacity = GROW_CAPACITY(oldCap);
        chunk->lines = GROW_ARRAY(chunk->lines, LineInfo, oldCap, chunk->lineCapacity);
    }
    LineInfo* lineInfo = &chunk->lines[chunk->lineCount++];
    lineInfo->offset = chunk->count - 1;
    lineInfo->line = line;
}

int getLine(Chunk* chunk, int instr) {
    int start = 0, end = chunk->lineCount - 1;
    while (end > start) {
        int mid = (start + end) / 2;
        LineInfo* lineInfo = &chunk->lines[mid];
        if (instr < lineInfo->offset) {
            end = mid - 1;
        } else if (mid == chunk->lineCount - 1 || instr < chunk->lines[mid + 1].offset) {
            return lineInfo->line;
        } else {
            start = mid + 1;
        }
    }
    return chunk->lines[start].line;
}

int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}
