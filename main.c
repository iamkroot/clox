#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

static void repl() {
    char line[1024];
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        interpret(line);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file '%s'.\n", path);
        exit(74);
    }
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*) malloc(fileSize + 1);
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    switch (result) {
        case INTERPRET_COMPILE_ERROR: exit(65);
        case INTERPRET_RUNTIME_ERROR: exit(70);
        default:
            break;
    }
}

int main(int argc, const char* argv[]) {
    initVM();
    switch (argc) {
        case 1:
            repl();
            break;
        case 2:
            runFile(argv[1]);
            break;
        default:
            fprintf(stderr, "Usage: clox [path]\n");
            exit(64);
    }
    freeVM();
    return 0;
}
