
#include "vm.hpp"
#include "chunk.hpp"
#include "debug.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>


static void repl() {
    Vm vm;
    
    for(;;){
        std::cout << "> ";
        std::string str;
        std::getline(std::cin, str);

        if( str == "exit" ){
            return;
        }

        vm.interpret(str.c_str());
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

    // if (result == INTERPRET_COMPILE_ERROR) exit(65);
    // if (result == INTERPRET_RUNTIME_ERROR) exit(70);
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
