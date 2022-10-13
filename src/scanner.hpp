#pragma once

#include <stdint.h>

/**
 * TODO streaming not load whole thing into a buffer
*/

struct Token {
    enum Type {
        // Single-character tokens:
        LEFT_PAREN, RIGHT_PAREN,
        LEFT_BRACE, RIGHT_BRACE,
        COMMA, MINUS, PLUS,
        SEMICOLON, SLASH, STAR,
        // One or two character tokens:
        BANG, BANG_EQUAL,
        EQUAL, EQUAL_EQUAL,
        GREATER, GREATER_EQUAL,
        LESS, LESS_EQUAL,
        // Literals:
        IDENTIFIER, STRING, NUMBER,   // TODO int
        // Keywords:
        AND, ELSE, FALSE,
        FOR, FN, IF, NIL, OR,
        PRINT, RETURN,
        TRUE, VAR, WHILE,
        // Special tokens:
        ERROR, END
    };

    Type type;
    char const * start;
    uint16_t length;
    uint16_t line;
};

class Scanner {
public:
    Scanner();
    
    ~Scanner();

    void init(char const * source);
    
    Token scanToken();

    static uint16_t const MAX_LINES = 0xFFFF;

private:
    inline bool isAtEnd_(){ return *current_ == '\0'; }
    inline char peek_(){ return *current_; }
    inline char advance_(){ return *current_++; }
    inline bool isDigit_(char c){ return c >= '0' && c <= '9'; }
    inline bool isAlpha_(char c){ 
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
                c == '_';
    }

    void incrementLine_();
    void skipWhitespace_();
    bool matchNext_(char expected);
    
    Token makeToken_(Token::Type type);
    Token makeErrorToken_(const char* message);
    Token makeStringToken_();
    Token makeNumberToken_();
    Token makeIdentifierToken_();
    Token::Type identifierType_();
    Token::Type checkKeyword_(int offset, int len, char const * rest, Token::Type type);
    

    char const * start_;
    char const * current_;
    uint16_t line_;
};
