
#include "compiler.hpp"
#include "scanner.hpp"
#include "debug.hpp"

#include <stdio.h>
#include <stdlib.h>


bool Compiler::compile(char const * source, Chunk & chunk) {
    hadError_ = false;
    panicMode_ = false;

    scanner_.init(source);
    
    advance_();
    expression_();
    consume_(Token::END, "Expect end of expression");
    
    return !hadError_;
}

void Compiler::advance_() {
    // record last token
    previous_ = current_;

    // spin until we get a valid token (or END):
    for(;;) {
        current_ = scanner_.scanToken();
        if( current_.type == Token::ERROR ){
            // report error then ignore and continue
            errorAtCurrent_(current_.start);
        }else{
            // valid token
            return;
        }
    }
}

void Compiler::consume_(Token::Type type, const char* message) {
    if( current_.type == type ){
        advance_();
        return;
    }
    errorAtCurrent_(message);
}

void Compiler::errorAtCurrent_(const char* message) {
    errorAt_(&current_, message);
}

void Compiler::error_(const char* message) {
  errorAt_(&previous_, message);
}

void Compiler::errorAt_(Token* token, const char* message) {
    if( panicMode_ ) return;  // suppress errors after the first
    panicMode_ = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == Token::END) {
        fprintf(stderr, " at end");
    } else if (token->type == Token::ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    hadError_ = true;
}

