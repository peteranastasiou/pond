
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
    emitByteAtLine_(byte, previousToken_.line);
}

void Compiler::emitByteAtLine_(uint8_t byte, uint16_t line) {
    currentChunk_()->write(byte, line);
}

void Compiler::endCompilation_() {
    emitReturn_();
}

void Compiler::emitReturn_() {
    emitByte_(OpCode::RETURN);
}

void Compiler::emitTrue_() {
    emitByte_(OpCode::TRUE);
}

void Compiler::emitFalse_() {
    emitByte_(OpCode::FALSE);
}

void Compiler::emitNil_() {
    emitByte_(OpCode::NIL);
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
    // Next token
    advance_();

    // Perform prefix rule of the token first:
    auto prefixRule = getRule_(previousToken_.type)->prefix;
    if( prefixRule == NULL ){
        errorAtPrevious_("Expect expression");
        return;
    }
    prefixRule();

    // Perforce infix rules on tokens from left to right:
    for( ;; ){
        ParseRule const * rule = getRule_(currentToken_.type);
        if( rule->precedence < precedence ){
            // Stop: the new token has lower precedence so is not part of the current operand
            break;
        }
        // Consume and then compile the operator:
        advance_();
        rule->infix();  // Can't be NULL as Precedence > NONE (refer getRule_ table)
    }
}

void Compiler::grouping_() {
    // The opening '( is already consumed, expect an expression next:
    expression_();

    // consume the closing brace:
    consume_(Token::RIGHT_PAREN, "Expect ')' after expression");
}

void Compiler::unary_() {
    Token::Type operatorType = previousToken_.type;
    uint16_t line = previousToken_.line;

    // Compile the operand evaluation first:
    parse_(Precedence::UNARY);

    // Result of the operand gets negated:
    switch( operatorType ){
        case Token::BANG:  emitByteAtLine_(OpCode::NOT, line); break;
        case Token::MINUS: emitByteAtLine_(OpCode::NEGATE, line); break;
        default: break;
    }
}

void Compiler::binary_() {
    // infix operator just got consumed, next token is the start of the second operand
    // the first operand is already compiled and will end up on the stack first
    Token::Type operatorType = previousToken_.type;
    ParseRule const * rule = getRule_(operatorType);

    // parse the second operand, and stop when the precendence is equal or lower
    // stopping when precedence is equal causes math to be left associative: 1+2+3 = (1+2)+3
    parse_((Precedence)((int)rule->precedence + 1));

    // now both operand values will end up on the stack. combine them:
    switch( operatorType ){
        case Token::BANG_EQUAL:    emitByte_(OpCode::NOT_EQUAL); break;
        case Token::EQUAL_EQUAL:   emitByte_(OpCode::EQUAL); break;
        case Token::GREATER:       emitByte_(OpCode::GREATER); break;
        case Token::GREATER_EQUAL: emitByte_(OpCode::GREATER_EQUAL); break;
        case Token::LESS:          emitByte_(OpCode::LESS); break;
        case Token::LESS_EQUAL:    emitByte_(OpCode::LESS_EQUAL); break;
        case Token::PLUS:          emitByte_(OpCode::ADD); break;
        case Token::MINUS:         emitByte_(OpCode::SUBTRACT); break;
        case Token::STAR:          emitByte_(OpCode::MULTIPLY); break;
        case Token::SLASH:         emitByte_(OpCode::DIVIDE); break;
        default: break;
    }
}

void Compiler::number_() {
    // shouldn't fail as we already validated the token as a number:
    double n = strtod(previousToken_.start, nullptr);
    emitConstant_(Value::number(n));
}

void Compiler::string_() {
    emitConstant_(Value::string(previousToken_.start+1, previousToken_.length-2));
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
        [Token::BANG]          = {RULE(unary_),    NULL,          Precedence::NONE},
        [Token::BANG_EQUAL]    = {NULL,            RULE(binary_), Precedence::EQUALITY},
        [Token::EQUAL]         = {NULL,            NULL,          Precedence::NONE},
        [Token::EQUAL_EQUAL]   = {NULL,            RULE(binary_), Precedence::EQUALITY},
        [Token::GREATER]       = {NULL,            RULE(binary_), Precedence::COMPARISON},
        [Token::GREATER_EQUAL] = {NULL,            RULE(binary_), Precedence::COMPARISON},
        [Token::LESS]          = {NULL,            RULE(binary_), Precedence::COMPARISON},
        [Token::LESS_EQUAL]    = {NULL,            RULE(binary_), Precedence::COMPARISON},
        [Token::IDENTIFIER]    = {NULL,            NULL,          Precedence::NONE},
        [Token::STRING]        = {RULE(string_),   NULL,          Precedence::NONE},
        [Token::NUMBER]        = {RULE(number_),   NULL,          Precedence::NONE},
        [Token::AND]           = {NULL,            NULL,          Precedence::NONE},
        [Token::ELSE]          = {NULL,            NULL,          Precedence::NONE},
        [Token::FALSE]         = {RULE(emitFalse_),NULL,          Precedence::NONE},
        [Token::FOR]           = {NULL,            NULL,          Precedence::NONE},
        [Token::FN]            = {NULL,            NULL,          Precedence::NONE},
        [Token::IF]            = {NULL,            NULL,          Precedence::NONE},
        [Token::NIL]           = {RULE(emitNil_),  NULL,          Precedence::NONE},
        [Token::OR]            = {NULL,            NULL,          Precedence::NONE},
        [Token::PRINT]         = {NULL,            NULL,          Precedence::NONE},
        [Token::RETURN]        = {NULL,            NULL,          Precedence::NONE},
        [Token::TRUE]          = {RULE(emitTrue_), NULL,          Precedence::NONE},
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

    fprintf(stderr, "%d: Error", token->line);

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

