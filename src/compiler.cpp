
#include "compiler.hpp"
#include "debug.hpp"

#include <stdio.h>
#include <stdlib.h>


Compiler::Compiler() {

}

Compiler::~Compiler() {

}

bool Compiler::compile(char const * source, Chunk & chunk) {
    scanner_.init(source);
    compilingChunk_ = &chunk;

    hadError_ = false;
    panicMode_ = false;
    
    advance_();
    expression_();
    consume_(Token::END, "Expect end of expression");
    
    endCompilation_();
    return !hadError_;
}

void Compiler::advance_() {
    // record last token
    previousToken_ = currentToken_;

    // spin until we get a valid token (or END):
    for(;;) {
        currentToken_ = scanner_.scanToken();
        if( currentToken_.line == Scanner::MAX_LINES ){
            fprintf(stderr, "Too many lines");
            exit(1); // TODO proper error handling
        }

        if( currentToken_.type == Token::ERROR ){
            // report error then ignore and continue
            errorAtCurrent_(currentToken_.start);
        }else{
            // valid token
            return;
        }
    }
}

void Compiler::consume_(Token::Type type, const char* message) {
    // Asserts that the current token is the type specified
    if( currentToken_.type == type ){
        // only advance if token is correct
        advance_();
        return;
    }
    errorAtCurrent_(message);
}

Chunk * Compiler::currentChunk_() {
    return compilingChunk_;
}

void Compiler::emitByte_(uint8_t byte) {
    currentChunk_()->write(byte, previousToken_.line);
}

void Compiler::endCompilation_() {
    emitReturn_();
}

void Compiler::emitReturn_() {
    emitByte_(OpCode::RETURN);
}

void Compiler::emitConstant_(Value value) {
    emitBytes_(OpCode::CONSTANT, makeConstant_(value));
}

uint8_t Compiler::makeConstant_(Value value) {
    uint8_t constant = currentChunk_()->addConstant(value);
    if( constant == Chunk::MAX_CONSTANTS ){
        errorAtPrevious_("Too many constants in one chunk.");
        return 0;
    }
    return constant;
}

void Compiler::expression_() {

}

void Compiler::grouping_() {
    expression_();
    consume_(Token::RIGHT_PAREN, "Expect ')' after expression");
}

void Compiler::number_() {
    // shouldn't fail as we already validated the token as a number:
    double value = strtod(previousToken_.start, nullptr);
    emitConstant_(value);
}

void Compiler::errorAtCurrent_(const char* message) {
    errorAt_(&currentToken_, message);
}

void Compiler::errorAtPrevious_(const char* message) {
  errorAt_(&previousToken_, message);
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

