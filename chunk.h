//
// Created by kroot on 15/7/19.
//

#ifndef CLOX_CHUNK_H
#define CLOX_CHUNK_H

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_RETURN
} OpCode;

typedef struct {
    int line;
    int offset;
} LineInfo;

typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int lineCount;
    int lineCapacity;
    LineInfo* lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int getLine(Chunk* chunk, int instr);
int addConstant(Chunk* chunk, Value value);

#endif //CLOX_CHUNK_H
