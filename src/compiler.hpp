#pragma once

#include "chunk.hpp"

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
    void advance_();
    void consume_(Token::Type type, const char* message);

    void errorAtCurrent_(const char* message);
    void error_(const char* message);  // TODO rename?
    void errorAt_(Token* token, const char* message);

    Scanner scanner_;
    Token current_;
    Token previous_;
    bool hadError_;
    bool panicMode_;
};