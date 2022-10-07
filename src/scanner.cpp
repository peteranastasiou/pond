
#include "scanner.hpp"

#include "string.h"

Scanner::Scanner(char const * source) {
    start_ = source;
    current_ = source;
    line_ = 1;
}

Scanner::~Scanner() {
}

void Scanner::skipWhitespace_() {
    for(;;){
        switch( peek_() ){
            case '\n':
                line_++;
                // Fall-through
            case ' ':
            case '\r':
            case '\t':
                advance_();
                break;

            case '#':
                // comment out the rest of the line:
                while( peek_()!='\n' && !isAtEnd_() ){
                    advance_();
                }

            default:
                return;
        }
    }
}

bool Scanner::matchNext_(char expected) {
    if( isAtEnd_() ) return false;  // EOF

    if( *current_ == expected ){
        current_++;
        return true;
    }
    return false;
}

Token Scanner::makeToken_(Token::Type type) {
  Token token;
  token.type = type;
  token.start = start_;
  token.length = (int)(current_ - start_);
  token.line = line_;
  return token;
}

Token Scanner::makeErrorToken_(const char* message) {
  Token token;
  token.type = Token::ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = line_;
  return token;
}

Token Scanner::makeStringToken_() {
    while( peek_() != '"' && !isAtEnd_() ){
        if( peek_() == '\n' ) line_++;
        advance_();
    }

    if( isAtEnd_() ) return makeErrorToken_("Unterminated string");

    // consume the closing quote:
    advance_();
    return makeToken_(Token::STRING);
}

Token Scanner::makeIdentifierToken_() {
    while( isAlpha_(peek_()) || isDigit_(peek_()) ){
        advance_();
    }
    return makeToken_(Token::IDENTIFIER);
}

Token Scanner::makeNumberToken_() {
    while( isDigit_(peek_()) ){
        advance_();
    }

    // Look for a fractional part.
    if( peek_() == '.' ) {
        // Consume the ".".
        advance_();

        // Must have a digit following '.':
        if( !isDigit_(peek_()) ){
            return makeErrorToken_("Malformed number");
        }

        while( isDigit_(peek_()) ){
            advance_();
        }
    }

    return makeToken_(Token::NUMBER);
}

Token Scanner::scanToken() {
    start_ = current_;

    // check for EOF:
    if( isAtEnd_() ) return makeToken_(Token::END);

    char c = advance_();
    if( isAlpha_(c) ) return makeIdentifierToken_();
    if( isDigit_(c) ) return makeNumberToken_();
    switch (c) {
        case '(': return makeToken_(Token::LEFT_PAREN);
        case ')': return makeToken_(Token::RIGHT_PAREN);
        case '{': return makeToken_(Token::LEFT_BRACE);
        case '}': return makeToken_(Token::RIGHT_BRACE);
        case ';': return makeToken_(Token::SEMICOLON);
        case ',': return makeToken_(Token::COMMA);
        case '.': return makeToken_(Token::DOT);
        case '-': return makeToken_(Token::MINUS);
        case '+': return makeToken_(Token::PLUS);
        case '/': return makeToken_(Token::SLASH);
        case '*': return makeToken_(Token::STAR);
        case '!': return makeToken_(matchNext_('=') ? Token::BANG_EQUAL : Token::BANG);
        case '=': return makeToken_(matchNext_('=') ? Token::EQUAL_EQUAL : Token::EQUAL);
        case '<': return makeToken_(matchNext_('=') ? Token::LESS_EQUAL : Token::LESS);
        case '>': return makeToken_(matchNext_('=') ? Token::GREATER_EQUAL : Token::GREATER);
        case '"': return makeStringToken_();
    }

    return makeErrorToken_("Unexpected character.");
}
