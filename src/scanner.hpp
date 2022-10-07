#pragma once

struct Token {
    enum Type {
        // Single-character tokens:
        LEFT_PAREN, RIGHT_PAREN,
        LEFT_BRACE, RIGHT_BRACE,
        COMMA, DOT, MINUS, PLUS,
        SEMICOLON, SLASH, STAR,
        // One or two character tokens:
        BANG, BANG_EQUAL,
        EQUAL, EQUAL_EQUAL,
        GREATER, GREATER_EQUAL,
        LESS, LESS_EQUAL,
        // Literals:
        IDENTIFIER, STRING, NUMBER,
        // Keywords:
        AND, CLASS, ELSE, FALSE,
        FOR, FUN, IF, NIL, OR,
        PRINT, RETURN, SUPER, THIS,
        TRUE, VAR, WHILE,
        // Special tokens:
        ERROR, END
    };

    Type type;
    char const * start;
    int length;
    int line;
};

class Scanner {
public:
    Scanner(char const * source);
    
    ~Scanner();

    Token scanToken();

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

    bool matchNext_(char expected);
    
    Token makeToken_(Token::Type type);
    Token makeErrorToken_(const char* message);
    Token makeStringToken_();
    Token makeNumberToken_();
    Token makeIdentifierToken_();
    
    void skipWhitespace_();

    char const * start_;
    char const * current_;
    int line_;
};
