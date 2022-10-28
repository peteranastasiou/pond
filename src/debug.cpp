
#include "debug.hpp"

#include <stdio.h>
#include <stdlib.h>


Dissassembler::Dissassembler(){
}

Dissassembler::~Dissassembler(){
}

void Dissassembler::disassembleChunk(Chunk * chunk, char const * name){
    printf("== %s ==\n", name);

    // first line number
    int lineIdx = 0;
    int lineInstrCount = chunk->lines[lineIdx].count; // number of bytecode bytes per line

    for (int offset = 0; offset < chunk->count();) {
        int incr = disassembleInstruction_(chunk, offset, chunk->lines[lineIdx].line);
        offset += incr;
        if( lineInstrCount -= incr <= 0 ){
            // new line:
            lineIdx ++;
            lineInstrCount = chunk->lines[lineIdx].count;
        }
    }
}

int Dissassembler::disassembleInstruction(Chunk * chunk, int offset){
    // TODO decypher the line number
    return disassembleInstruction_(chunk, offset, 0);
}

int Dissassembler::disassembleInstruction_(Chunk * chunk, int offset, int line){
    printf("%04i ", offset);
    printf("%4d ", line);

    uint8_t instr = chunk->code[(size_t)offset];
    switch(instr){
        case OpCode::CONSTANT:      return constantInstruction_("CONSTANT", chunk, offset);
        case OpCode::NIL:           return simpleInstruction_("NIL");
        case OpCode::TRUE:          return simpleInstruction_("TRUE");
        case OpCode::FALSE:         return simpleInstruction_("FALSE");
        case OpCode::ADD:           return simpleInstruction_("ADD");
        case OpCode::EQUAL:         return simpleInstruction_("EQUAL"); 
        case OpCode::NOT_EQUAL:     return simpleInstruction_("NOT_EQUAL");     
        case OpCode::GREATER:       return simpleInstruction_("GREATER");   
        case OpCode::GREATER_EQUAL: return simpleInstruction_("GREATER_EQUAL");         
        case OpCode::LESS:          return simpleInstruction_("LESS"); 
        case OpCode::LESS_EQUAL:    return simpleInstruction_("LESS_EQUAL");  
        case OpCode::SUBTRACT:      return simpleInstruction_("SUBTRACT");
        case OpCode::MULTIPLY:      return simpleInstruction_("MULTIPLY");
        case OpCode::DIVIDE:        return simpleInstruction_("DIVIDE");
        case OpCode::NEGATE:        return simpleInstruction_("NEGATE");
        case OpCode::NOT:           return simpleInstruction_("NOT");
        case OpCode::RETURN:        return simpleInstruction_("RETURN");
        default:
            printf("Unknown opcode %i\n", instr);
            return 1;
    }
}

int Dissassembler::constantInstruction_(char const * name, Chunk * chunk, int offset){
    uint8_t constantIdx = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constantIdx);
    chunk->constants[constantIdx].print();
    printf("'\n");
    return 2;
}

int Dissassembler::simpleInstruction_(char const * name){
    printf("%s\n", name);
    return 1;
}

void debugScanner(char const * source) {
    Scanner scanner;
    scanner.init(source);
    int line = -1;
    for(;;){
        Token token = scanner.scanToken();
        if( token.line != line ){
            printf("%4d ", token.line);
            line = token.line;
        }else{
            printf("   | ");
        }
        printToken(token);
        printf("\n");
        if( token.type == Token::END ){
            break;
        }
    }
}

void printToken(Token token) {
    printf("%s '%.*s'", tokenTypeToStr(token.type), token.length, token.start); 
}

char const * tokenTypeToStr(Token::Type t) {
    switch(t) {           
        case Token::LEFT_PAREN:     return "LEFT_PAREN";
        case Token::RIGHT_PAREN:    return "RIGHT_PAREN";
        case Token::LEFT_BRACE:     return "LEFT_BRACE";
        case Token::RIGHT_BRACE:    return "RIGHT_BRACE";
        case Token::COMMA:          return "COMMA";
        case Token::MINUS:          return "MINUS";
        case Token::PLUS:           return "PLUS";
        case Token::SEMICOLON:      return "SEMICOLON";
        case Token::SLASH:          return "SLASH";
        case Token::STAR:           return "STAR";
        case Token::BANG:           return "BANG";
        case Token::BANG_EQUAL:     return "BANG_EQUAL";
        case Token::EQUAL:          return "EQUAL";
        case Token::EQUAL_EQUAL:    return "EQUAL_EQUAL";
        case Token::GREATER:        return "GREATER";
        case Token::GREATER_EQUAL:  return "GREATER_EQUAL";
        case Token::LESS:           return "LESS";
        case Token::LESS_EQUAL:     return "LESS_EQUAL";
        case Token::IDENTIFIER:     return "IDENTIFIER";
        case Token::STRING:         return "STRING";
        case Token::NUMBER:         return "NUMBER";
        case Token::AND:            return "AND";
        case Token::ELSE:           return "ELSE";
        case Token::FALSE:          return "FALSE";
        case Token::FOR:            return "FOR";
        case Token::FN:             return "FN";
        case Token::IF:             return "IF";
        case Token::NIL:            return "NIL";
        case Token::OR:             return "OR";
        case Token::PRINT:          return "PRINT";
        case Token::RETURN:         return "RETURN";
        case Token::TRUE:           return "TRUE";
        case Token::VAR:            return "VAR";
        case Token::WHILE:          return "WHILE";
        case Token::ERROR:          return "ERROR";
        case Token::END:            return "END";
        default:                    return "UNIDENTIFIED";
    }
}

void debugObjectLinkedList(Obj * obj) {
    printf("Objects:\n");
    while( obj != nullptr ){
        printf("  %p: [%s]\n", obj, obj->toString().c_str());
        obj = obj->next;
    }
}
