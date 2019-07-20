//
// Created by kroot on 19/7/19.
//

#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

#include "vm.h"

bool compile(const char* source, Chunk* chunk);

#endif //CLOX_COMPILER_H
