//
// Created by kroot on 17/7/19.
//
#include <stdio.h>

#include "memory.h"
#include "value.h"

void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->count >= array->capacity) {
        int oldCap = array->capacity;
        array->capacity = GROW_CAPACITY(oldCap);
        array->values = GROW_ARRAY(array->values, Value, oldCap, array->capacity);
    }
    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValue(Value value) {
    printf("%g", value);
}