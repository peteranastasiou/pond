#pragma once

#include "chunk.hpp"
#include "scanner.hpp"

class Compiler {
public:
    Compiler();

    ~Compiler();

    /**
     * @param source [input]
     * @param chunk [output]
    */
    bool compile(char const * source, Chunk & chunk);

private:
    // parser helpers:
    void advance_();
    void consume_(Token::Type type, const char* message);
    Chunk * currentChunk_();

    // parsing different types of things:
    void expression_();
    void number_();
    void grouping_();  // parentheses in expressions

    // bytecode helpers:
    void emitByte_(uint8_t byte);
    inline void emitBytes_(uint8_t b1, uint8_t b2){ emitByte_(b1); emitByte_(b2); }
    void endCompilation_();
    void emitReturn_();
    void emitConstant_(Value value);
    uint8_t makeConstant_(Value value);

    // error production:
    void errorAtCurrent_(const char* message);
    void errorAtPrevious_(const char* message);
    void errorAt_(Token* token, const char* message);

    Scanner scanner_;
    Chunk * compilingChunk_;
    Token currentToken_;
    Token previousToken_;
    bool hadError_;
    bool panicMode_;
};