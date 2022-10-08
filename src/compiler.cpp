
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
    parse_(Precedence::ASSIGNMENT);
}

void Compiler::parse_(Precedence precedence) {

}

void Compiler::grouping_() {
    expression_();
    consume_(Token::RIGHT_PAREN, "Expect ')' after expression");
}

void Compiler::unary_() {
}

void Compiler::binary_() {
}

void Compiler::number_() {
    // shouldn't fail as we already validated the token as a number:
    double value = strtod(previousToken_.start, nullptr);
    emitConstant_(value);
}

#define RULE(fn) [this](){ this->fn(); }

ParseRule const * Compiler::getRule_(Token::Type type) {
    static const ParseRule rules[] = {
        // token type             prefix func      infix func     infix precedence
        [Token::LEFT_PAREN]    = {RULE(grouping_), NULL,          Precedence::NONE},
        [Token::RIGHT_PAREN]   = {NULL,            NULL,          Precedence::NONE},
        [Token::LEFT_BRACE]    = {NULL,            NULL,          Precedence::NONE}, 
        [Token::RIGHT_BRACE]   = {NULL,            NULL,          Precedence::NONE},
        [Token::COMMA]         = {NULL,            NULL,          Precedence::NONE},
        [Token::MINUS]         = {RULE(unary_),    RULE(binary_), Precedence::TERM},
        [Token::PLUS]          = {NULL,            RULE(binary_), Precedence::TERM},
        [Token::SEMICOLON]     = {NULL,            NULL,          Precedence::NONE},
        [Token::SLASH]         = {NULL,            RULE(binary_), Precedence::FACTOR},
        [Token::STAR]          = {NULL,            RULE(binary_), Precedence::FACTOR},
        [Token::BANG]          = {NULL,            NULL,          Precedence::NONE},
        [Token::BANG_EQUAL]    = {NULL,            NULL,          Precedence::NONE},
        [Token::EQUAL]         = {NULL,            NULL,          Precedence::NONE},
        [Token::EQUAL_EQUAL]   = {NULL,            NULL,          Precedence::NONE},
        [Token::GREATER]       = {NULL,            NULL,          Precedence::NONE},
        [Token::GREATER_EQUAL] = {NULL,            NULL,          Precedence::NONE},
        [Token::LESS]          = {NULL,            NULL,          Precedence::NONE},
        [Token::LESS_EQUAL]    = {NULL,            NULL,          Precedence::NONE},
        [Token::IDENTIFIER]    = {NULL,            NULL,          Precedence::NONE},
        [Token::STRING]        = {NULL,            NULL,          Precedence::NONE},
        [Token::NUMBER]        = {RULE(number_),   NULL,          Precedence::NONE},
        [Token::AND]           = {NULL,            NULL,          Precedence::NONE},
        [Token::ELSE]          = {NULL,            NULL,          Precedence::NONE},
        [Token::FALSE]         = {NULL,            NULL,          Precedence::NONE},
        [Token::FOR]           = {NULL,            NULL,          Precedence::NONE},
        [Token::FN]            = {NULL,            NULL,          Precedence::NONE},
        [Token::IF]            = {NULL,            NULL,          Precedence::NONE},
        [Token::NIL]           = {NULL,            NULL,          Precedence::NONE},
        [Token::OR]            = {NULL,            NULL,          Precedence::NONE},
        [Token::PRINT]         = {NULL,            NULL,          Precedence::NONE},
        [Token::RETURN]        = {NULL,            NULL,          Precedence::NONE},
        [Token::TRUE]          = {NULL,            NULL,          Precedence::NONE},
        [Token::VAR]           = {NULL,            NULL,          Precedence::NONE},
        [Token::WHILE]         = {NULL,            NULL,          Precedence::NONE},
        [Token::ERROR]         = {NULL,            NULL,          Precedence::NONE},
        [Token::END]           = {NULL,            NULL,          Precedence::NONE},
    };
    return &rules[type];
}
#undef RULE

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

