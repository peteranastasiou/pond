
#include "compiler.hpp"
#include "scanner.hpp"
#include "debug.hpp"

#include <stdio.h>


void compile(char const * source) {
    Scanner scanner(source);
    int line = -1;
    for(;;){
        Token token = scanner.scanToken();
        if( token.line != line ){
            printf("%4d ", token.line);
            line = token.line;
        }else{
            printf("   | ");
        }
        printf("%s '%.*s'\n", tokenTypeToStr(token.type), token.length, token.start); 

        if( token.type == Token::END ){
            break;
        }
    }
}
