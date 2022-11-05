
#include "vm.hpp"
#include "chunk.hpp"
#include "debug.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>


static void repl() {
    Vm vm;

    // TODO tab completion!
    // rl_completion_matches = autocomplete;  // ref https://eli.thegreenplace.net/2016/basics-of-using-the-readline-library/

    for( ;; ){
        char * line = readline("> ");
        if( line == nullptr ) return;  // Ctrl C or D

        if( strlen(line) > 0 ){
            add_history(line);
        }
        
        vm.interpret(line);

        free(line);
    }
}

static char* readFile(const char* path) {
    // todo read in file as scanned instead of loading the whole thing into memory!
    FILE* file = fopen(path, "rb");
    if( file == NULL ){
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    Vm vm;
    char* source = readFile(path);
    InterpretResult result = vm.interpret(source);
    free(source);

    if (result == InterpretResult::COMPILE_ERR) exit(65);
    if (result == InterpretResult::RUNTIME_ERR) exit(70);
}

int main(int argc, char const * argv[]) {
    if( argc == 1 ){
        repl();
    }else if( argc == 2 ){
        runFile(argv[1]);
    }else{
        fprintf(stderr, "Usage: pond [path]\n");
        return 64;
    }

    return 0;
}
