
#include "scanner.hpp"

#include "string.h"

Scanner::Scanner() {
}

Scanner::~Scanner() {
}

void Scanner::init(char const * source) {
    start_ = source;
    current_ = source;
    line_ = 1;
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
    // TODO string escape characters
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
    return makeToken_(identifierType_());
}

Token::Type Scanner::identifierType_() {
    // Use a trie to determine if the identifier is a keyword:
    switch( start_[0] ){
        case 'a': return checkKeyword_(1, 2, "nd", Token::AND);
        case 'f': {
            // "f..." might be "false", "for" or "fn":
            // first check if identifier is longer than 1 char:
            if( current_ - start_ > 1 ){
                switch( start_[1] ){
                    case 'a': return checkKeyword_(2, 3, "lse", Token::FALSE);
                    case 'o': return checkKeyword_(2, 1, "r", Token::FALSE);
                    case 'n': return Token::FN;
                }
            }
            break;
        }
        case 'e': return checkKeyword_(1, 3, "lse", Token::ELSE);
        case 'i': return checkKeyword_(1, 1, "f", Token::IF);
        case 'n': return checkKeyword_(1, 2, "il", Token::NIL);
        case 'o': return checkKeyword_(1, 1, "r", Token::OR);
        case 'p': return checkKeyword_(1, 4, "rint", Token::PRINT);
        case 'r': return checkKeyword_(1, 5, "eturn", Token::RETURN);
        case 't': return checkKeyword_(1, 3, "rue", Token::TRUE);
        case 'v': return checkKeyword_(1, 2, "ar", Token::VAR);
        case 'w': return checkKeyword_(1, 4, "hile", Token::WHILE);
    }
    // Not a keyword:
    return Token::IDENTIFIER;
}

Token::Type Scanner::checkKeyword_(int offset, int len, char const * rest, Token::Type type) {
    // check length is correct:
    if( current_ - start_ == offset + len ){
        // check string matches:
        if( memcmp(start_ + offset, rest, len) == 0 ){
            return type;
        }
    }
    // Not a keyword:
    return Token::IDENTIFIER;
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
    // first, gobble up whitespace and comments:
    skipWhitespace_();

    // reset pointer to the start of identifier/keyword:
    start_ = current_;

    // check for EOF:
    if( isAtEnd_() ) return makeToken_(Token::END);

    char c = advance_();

    // check for indentifier/keyword:
    if( isAlpha_(c) ) return makeIdentifierToken_();

    // check for number:
    if( isDigit_(c) ) return makeNumberToken_();

    // check for symbol:
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
