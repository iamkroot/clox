//
// Created by kroot on 15/7/19.
//

#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "vm.h"

#ifdef DEBUG_LOG_GC

#include <stdio.h>
#include "debug.h"

#endif

void* reallocate(void* previous, size_t oldSize, size_t newSize) {
    if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
        collectGarbage();
#endif
    }
    if (newSize == 0) {
        free(previous);
        return NULL;
    }
    return realloc(previous, newSize);
}

void markObject(Obj* obj) {
    if (obj == NULL || obj->isMarked)
        return;
#ifdef DEBUG_LOG_GC
    printf("%p mark ", (void*)obj);
    printValue(OBJ_VAL(obj));
    printf("\n");
#endif
    obj->isMarked = true;
    if (vm.grayCapacity < vm.grayCount + 1) {
        vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
        vm.grayStack = realloc(vm.grayStack, sizeof(Obj*) * vm.grayCapacity);
    }
    vm.grayStack[vm.grayCount++] = obj;
}


void markValue(Value value) {
    if (!IS_OBJ(value)) return;
    markObject(AS_OBJ(value));
}

static void markArray(ValueArray* array) {
    for (int i = 0; i < array->count; ++i) {
        markValue(array->values[i]);
    }
}

static void blackenObject(Obj* obj) {
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void*)obj);
    printValue(OBJ_VAL(obj));
    printf("\n");
#endif
    switch (obj->type) {
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)obj;
            markObject((Obj*)closure->function);
            for (int i = 0; i < closure->upvalueCount; ++i) {
                markObject((Obj*)closure->upvalues[i]);
            }
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)obj;
            markObject((Obj*)function->name);
            markArray(&function->chunk.constants);
            break;
        }
        case OBJ_UPVALUE: {
            markValue(((ObjUpvalue*)obj)->closed);
            break;
        }
        case OBJ_NATIVE:
        case OBJ_STRING: break;
    }
}

static void traceRefs() {
    while (vm.grayCount > 0) {
        Obj* obj = vm.grayStack[--vm.grayCount];
        blackenObject(obj);
    }
}

static void markRoots() {
    for (Value* slot = 0; slot < vm.stackTop; ++slot) {
        markValue(*slot);
    }
    for (int i = 0; i < vm.frameCount; ++i) {
        markObject((Obj*)vm.frames[i].closure);
    }
    for (ObjUpvalue* upvalue = vm.openUpvalues; upvalue != NULL; upvalue = upvalue->next) {
        markObject((Obj*)upvalue);
    }
    markTable(&vm.globals);
    markCompilerRoots();
}

void collectGarbage() {
#ifdef  DEBUG_LOG_GC
    printf("-- gc begin\n");
#endif
    markRoots();
    traceRefs();
#ifdef  DEBUG_LOG_GC
    printf("-- gc end\n");
#endif
}

static void freeObject(Obj* obj) {
#ifdef DEBUG_LOG_GC
    printf("%p free type %d\n", (void*)obj, obj->type);
#endif
    switch (obj->type) {
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)obj;
            FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalueCount);
            FREE(ObjClosure, obj);
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)obj;
            freeChunk(&function->chunk);
            FREE(ObjFunction, obj);
            break;
        }
        case OBJ_NATIVE: {
            FREE(ObjNative, obj);
            break;
        }
        case OBJ_STRING: {
            ObjString* string = (ObjString*)obj;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(ObjString, obj);
            break;
        }
        case OBJ_UPVALUE: {
            FREE(ObjUpvalue, obj);
            break;
        }
    }
}

void freeObjects() {
    Obj* obj = vm.objects;
    while (obj != NULL) {
        Obj* next = obj->next;
        freeObject(obj);
        obj = next;
    }
    free(vm.grayStack);
}