
#include "compiler.hpp"
#include "debug.hpp"
#include "vm.hpp"
#include "function.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <vector>


Environment::Environment(Vm * vm, ObjString * name, Type t) {
    type = t;
    localCount = 0;
    scopeDepth = 0;
    function = new ObjFunction(vm, name);

    // Claim first local for the "stack pointer"??
    Local * local = &locals[localCount++];
    local->depth = 0;
    local->name.start = "";
    local->name.length = 0;
}


bool Environment::addLocal(Token & name, bool isConst) {
    if( localCount == MAX_LOCALS ){
        return false;
    }

    Local * local = &locals[localCount++];
    local->name = name;
    local->depth = scopeDepth;
    local->isDefined = false;
    local->isConst = isConst;
    return true;
}

void Environment::defineLocal() {
    // Mark local as having a value now:
    locals[localCount-1].isDefined = true;
}

int Environment::resolveLocal(Compiler * c, Token & name, bool & isConst) {
    // search for a local by name in the environment
    // NOTE: searching from higher depth to lower, to support shadowing correctly
    for( int i = localCount-1; i >= 0; i-- ){
        Local * local = &locals[i];
        if( name.equals(local->name) ){
            // Handle special case where it hasn't been initialised before reference
            // e.g. var a = a;
            if( !local->isDefined ){
                c->errorAtPrevious_("Local variable referenced before definition.");
            }

            // Found it:
            isConst = local->isConst;
            // The local index happens to also be its position on the stack at runtime:
            return i;
        }
    }
    return Local::NOT_FOUND;
}

int Environment::resolveUpvalue(Compiler * c, Token & name, bool & isConst) {
    // can't check enclosing env if already top-level:
    if( enclosing == nullptr ) return Local::NOT_FOUND;

    // search for local in enclosing environment/function:
    int local = enclosing->resolveLocal(c, name, isConst);
    if( local != Local::NOT_FOUND ){
        return addUpvalue(c, (uint8_t)local, isConst, true);
    }

    // search for upvalue in enclosing environment/function:
    int upvalue = enclosing->resolveUpvalue(c, name, isConst);
    if( upvalue != Local::NOT_FOUND ){
        return addUpvalue(c, (uint8_t)upvalue, isConst, false);
    }

    return Local::NOT_FOUND;
}

int Environment::addUpvalue(Compiler * c, uint8_t index, bool isConst, bool isLocal) {
    int n = function->numUpvalues;

    for( int i = 0; i < n; i++ ){
        Upvalue * upvalue = &upvalues[i];
        if( upvalue->index == index && upvalue->isLocal == isLocal ){
            return i; // already got it!
        }
    }

    if( n == MAX_UPVALUES ){
        c->errorAtPrevious_("Too many closure variables in function.");
        return 0;
    }

    upvalues[n] = {.index=index, .isConst=isConst, .isLocal=isLocal};
    return function->numUpvalues++;
}

uint8_t Environment::freeLocals() {
    uint8_t nFreed = 0;
    // remove all locals which have fallen out of scope:
    while( localCount > 0 && locals[localCount-1].depth > scopeDepth ){
        localCount--;
        nFreed++;
    }
    return nFreed;
}

Compiler::Compiler(Vm * vm) : vm_(vm) {
}

Compiler::~Compiler() {
}

ObjFunction * Compiler::compile(char const * source) {
    scanner_.init(source);

    currentEnv_ = nullptr;
    Environment env(vm_, ObjString::newString(vm_, "<script>"), Environment::SCRIPT);
    initEnvironment_(env);

    hadError_ = false;
    hadFatalError_ = false;
    panicMode_ = false;

    advance_();  // get the first token
    
    // compile declarations until we hit the end
    while( !match_(Token::END) ){
        declaration_(false);

        if( hadFatalError_ ) break;
    }

    ObjFunction * function = endEnvironment_();
    return hadError_ ? nullptr : function;
}

void Compiler::initEnvironment_(Environment & env) {
    env.enclosing = currentEnv_;
    currentEnv_ = &env;
}

ObjFunction * Compiler::endEnvironment_() {
    emitReturn_();
    ObjFunction * fn = currentEnv_->function;
    currentEnv_ = currentEnv_->enclosing;
    return fn;
}

void Compiler::advance_() {
    // record last token
    previousToken_ = currentToken_;
    // spin until we get a valid token (or END):
    for(;;) {
        currentToken_ = scanner_.scanToken();
        if( currentToken_.line == Scanner::MAX_LINES ){
            fatalError_("Too many lines in script");
            // pretend this is the end of the script
            currentToken_.type = Token::END;
            return;
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

bool Compiler::check_(Token::Type type) {
    return currentToken_.type == type;
}

void Compiler::consume_(Token::Type type, const char* fmt, ...) {
    // Asserts that the current token is the type specified
    if( check_(type) ){
        // only advance if token is correct
        advance_();
        return;
    }
    va_list args;
    va_start(args, fmt);
    errorAtVargs_(&currentToken_, fmt, args);
    va_end(args);
}

bool Compiler::match_(Token::Type type) {
    // Like consume_ but return bool instead of throwing error
    if( check_(type) ){
        // only advance if token is correct
        advance_();
        return true;
    }
    return false;
}

Chunk * Compiler::getCurrentChunk_() {
    return &currentEnv_->function->chunk;
}

void Compiler::emitByte_(uint8_t byte) {
    emitByteAtLine_(byte, previousToken_.line);
}

void Compiler::emitByteAtLine_(uint8_t byte, uint16_t line) {
    if( !getCurrentChunk_()->write(byte, line) ){
        if( currentEnv_->type == Environment::FUNCTION ){
            fatalError_("Too much code in function.");
        }else{
            fatalError_("Too much code in top level of script.");
        }
    }
}

void Compiler::emitReturn_() {
    emitByte_(OpCode::NIL); // implicit return value
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

void Compiler::emitBoolType_() {
    emitByte_(OpCode::TYPE_BOOL);
}

void Compiler::emitFloatType_() {
    emitByte_(OpCode::TYPE_FLOAT);
}

void Compiler::emitObjectType_() {
    emitByte_(OpCode::TYPE_FUNCTION);
}

void Compiler::emitStringType_() {
    emitByte_(OpCode::TYPE_STRING);
}

void Compiler::emitTypeIdType_() {
    emitByte_(OpCode::TYPE_TYPEID);
}

void Compiler::emitLiteral_(Value value) {
    emitBytes_(OpCode::LITERAL, makeLiteral_(value));
}

uint8_t Compiler::makeLiteral_(Value value) {
    uint8_t literal = getCurrentChunk_()->addLiteral(value);
    if( literal == Chunk::MAX_LITERALS ){
        errorAtPrevious_("Too many literals in one chunk.");
        return 0;
    }
    return literal;
}

void Compiler::expression_() {
    parse_(Precedence::ASSIGNMENT);
}

bool Compiler::declaration_(bool canBeExpression) {
    bool isExpression = false;
    if( match_(Token::VAR) ){
        varDeclaration_(false);

    }else if( match_(Token::CONST) ){
        varDeclaration_(true);

    }else if( match_(Token::FN) ){
        funcDeclaration_();

    }else{
        isExpression = statement_(canBeExpression);
    }

    // End of a statement is a good place to re-sync the parser if it is panicking
    if( panicMode_ && !isExpression ) synchronise_();

    return isExpression;
}

void Compiler::funcDeclaration_() {
    bool isLocal = currentEnv_->scopeDepth > 0;
    bool isConst = true;  // Disallow redefining functions

    // Load the function variable name, getting the literals index (if global) or 0 (if local):
    uint8_t global = parseVariable_("Expected variable name.", isConst, isLocal);

    // capture function name for the environment too:
    ObjString * name = ObjString::newString(vm_, 
        previousToken_.start, previousToken_.length);

    // If its a local, mark it as already defined (allowing for self-referential functions):
    // This is not an issue for globals
    if( isLocal ) currentEnv_->defineLocal();

    // parse arguments and function content
    function_(name, Environment::FUNCTION);

    // assign function literal to variable
    defineVariable_(global, isConst, isLocal);
}

void Compiler::funcAnonymous_() {
    // Parse a function used in an expression: fn(args) { statements }
    function_(ObjString::newString(vm_, "(anon)"), Environment::FUNCTION);
}

void Compiler::function_(ObjString * name, Environment::Type type) {
    // new environment
    Environment env(vm_, name, type);
    initEnvironment_(env);
    beginScope_();

    // (i.e. function as expression)
    consume_(Token::LEFT_PAREN, "Expected '(' for function.");
    // if has any parameters:
    if( !check_(Token::RIGHT_PAREN) ){
        do {
            // count parameters
            if( ++env.function->numInputs > 255 ){
                errorAtCurrent_("Can't have over 255 parameters.");
            }
            // make a new local at the top of the function's value stack to use as the parameter:
            bool isLocal = true;
            bool isConst = false;
            parseVariable_("Expected parameter name.", isConst, isLocal);
            defineVariable_(0, isConst, isLocal);
        } while( match_(Token::COMMA) );
    }
    consume_(Token::RIGHT_PAREN, "Expected ')' after parameters.");
    consume_(Token::LEFT_BRACE, "Expected '{' before function body.");

    bool isExpression = block_(true);
    if( isExpression ){
        // function ends with an expression (omitted semi-colon)
        // to produce an implicit return:
        emitByte_(OpCode::RETURN);
    }

    // Note: no actual need to endScope(), as we are done with the Environment now
    // call it so that we can check the stack emptied correctly:
    endScope_();

    // New function literal:
    ObjFunction * fn = endEnvironment_();
    uint8_t literal = makeLiteral_(Value::function(fn));
    // Note: CLOSURE instruction takes a function literal and wraps it to make a Closure
    emitBytes_(OpCode::CLOSURE, literal);

    // List all the upvalues (variables enclosed by function):
    for( int i = 0; i < fn->numUpvalues; i++ ){
        // track whether it is a local or already an upvalue which is being uplifted:
        emitByte_(env.upvalues[i].isLocal ? 1 : 0);
        // stack position of value to lift:
        emitByte_(env.upvalues[i].index);
    }
}

void Compiler::varDeclaration_(bool isConst) {
    // local and global scoped variables are implemented differently:
    bool isLocal = currentEnv_->scopeDepth > 0;

    // Load the variable name, getting the literals index (if global) or 0 (if local):
    uint8_t global = parseVariable_("Expected variable name.", isConst, isLocal);

    // assigned an initial value?
    if( match_(Token::EQUAL) ){
        expression_();
    }else{
        emitByte_(OpCode::NIL); // default value is nil
    }
    consume_(Token::SEMICOLON, "Expected ';' after var declaration.");

    defineVariable_(global, isConst, isLocal);
}

uint8_t Compiler::parseVariable_(const char * errorMsg, bool isConst, bool isLocal) {
    // the name of the variable:
    consume_( Token::IDENTIFIER, errorMsg );

    if(isLocal) {
        // local variables are registered to the stack
        declareLocal_(isConst);
        return 0; // not a global
    } else {
        // globals variables have their names stored as a literal:
        return makeIdentifierLiteral_(previousToken_);
    }
}

void Compiler::declareLocal_(bool isConst) {
    // the name of the new local variable:
    Token * name = &previousToken_;

    // ensure the variable is not already declared in this scope!
    for( int i = currentEnv_->localCount-1; i>=0; i-- ){
        Local * local = &currentEnv_->locals[i];
        if( local->depth < currentEnv_->scopeDepth ){
            break;  // left the scope - stop searching
        }
        if( name->equals(local->name) ){
            errorAtPrevious_("Already a variable called '%.*s' in this scope.", 
                             name->length, name->start);
        }
    }

    // New local variable to track:
    if( !currentEnv_->addLocal(*name, isConst) ){
        errorAtPrevious_("Too many local variables in function.");
    }
}

void Compiler::defineVariable_(uint8_t global, bool isConst, bool isLocal) {
    if( isLocal ){
        currentEnv_->defineLocal();
    }else if( isConst ){
        emitBytes_(OpCode::DEFINE_GLOBAL_CONST, global);
    }else{
        emitBytes_(OpCode::DEFINE_GLOBAL_VAR, global);
    }
}

void Compiler::and_() {
    // left hand side has already been compiled, 
    // if its falsy, we want to jump over the right hand side (short circuiting)
    int jumpOverRhs = emitJump_(OpCode::JUMP_IF_FALSE);
    emitByte_(OpCode::POP);   // don't need the lhs anymore, if we got here - its true!
    parse_(Precedence::AND);  // the rhs value
    setJumpDestination_(jumpOverRhs);
}

void Compiler::or_() {
    // left hand side has already been compiled.
    // if its truthy, jump over the right hand side (short circuiting)
    int jumpOverRhs = emitJump_(OpCode::JUMP_IF_TRUE);
    emitByte_(OpCode::POP);  // don't need the lhs anymore
    parse_(Precedence::OR);  // the rhs value
    setJumpDestination_(jumpOverRhs);
}

bool Compiler::statement_(bool canBeExpression) {
    if( match_(Token::IF) ){
        return if_(canBeExpression);

    }else if( match_(Token::WHILE) ){
        whileStatement_();
        return false;  // statement only (for now!)

    }else if( match_(Token::LEFT_BRACE) ){
        // recurse into a nested scope:
        return nestedBlock_(canBeExpression);

    }else if( match_(Token::RETURN) ){
        if( currentEnv_->type == Environment::SCRIPT ){
            errorAtPrevious_("Can't return from top-level.");
        }
        if( check_(Token::SEMICOLON) ){
            emitReturn_();
        }else{
            // the return value(s):
            expression_();
            emitByte_(OpCode::RETURN);
        }
    }else{
        // expression-statement:
        expression_();
    }
    // what we expect next depends on the context of the expression-statement:
    if( !canBeExpression ){
        // ordinary statement:
        consume_(Token::SEMICOLON, "Expected ';' after statement.");
        emitByte_(OpCode::POP); // discard the result
        return false;
    }else if( match_(Token::SEMICOLON) ){
        // statement within an expression block:
        emitByte_(OpCode::POP); // discard the result
        return false;
    }else if( check_(Token::RIGHT_BRACE) ){
        // The end of an expression block, leave the value on the stack:
        return true;
    }else{
        errorAtCurrent_("Expected ';' or '}'.");
        return false;
    }
}

void Compiler::ifExpression_() {
    bool isExpression = if_(true);
    if( !isExpression ){
        errorAtPrevious_("Expected if-expression, not if-statement.");
    }
}

void Compiler::ifStatement_() {
    bool isExpression = if_(false);
    if( isExpression ){
        errorAtPrevious_("Expected if-statement, not if-expression.");
    }
}

bool Compiler::if_(bool canBeExpression) {
    // the condition part:
    expression_();
    // jump over the block to the next part:
    int jumpOver = emitJump_(OpCode::JUMP_IF_FALSE_POP);
    // the block
    consume_(Token::LEFT_BRACE, "Expected '{' after condition.");
    bool isExpression = nestedBlock_(canBeExpression);

    // track all the jumps which go straight to the end
    std::vector<int> jumpsToEnd;

    // optional `elif` blocks:
    while( match_(Token::ELIF) ){
        // protect against fallthrough
        jumpsToEnd.push_back(emitJump_(OpCode::JUMP));
        // jump over the previous if/elif-block to here:
        setJumpDestination_(jumpOver);
        // the condition part:
        expression_();
        // jump over the block to the next part:
        jumpOver = emitJump_(OpCode::JUMP_IF_FALSE_POP);
        // the block
        consume_(Token::LEFT_BRACE, "Expected '{' after 'elif'.");
        if( nestedBlock_(canBeExpression) != isExpression ){
            errorAtPrevious_("Inconsistent if-statement/if-expression.");
        }
    }

    // optional `else` block:
    if( match_(Token::ELSE) ){
        // protect against fallthrough
        jumpsToEnd.push_back(emitJump_(OpCode::JUMP));
        // jump over the previous if/elif-block to here:
        setJumpDestination_(jumpOver);
        // the block
        consume_(Token::LEFT_BRACE, "Expected '{' after 'else'.");
        if( nestedBlock_(canBeExpression) != isExpression ){
            errorAtPrevious_("Inconsistent if-statement/if-expression.");
        }
    }else{
        // no else block, so the last "jumpOver" goes to here:
        setJumpDestination_(jumpOver);

        if( isExpression ){
            errorAtPrevious_("Expected 'else' on if expression.");
        }
    }
    // link up all the end jumps to here
    for( int jump : jumpsToEnd ){
        setJumpDestination_(jump);
    }

    return isExpression;
}

void Compiler::whileStatement_() {
    // check the condition (this is where we loop):
    int loopStart = getCurrentChunk_()->count();
    expression_();
    // jump over the body if falsy
    int jumpToEnd = emitJump_(OpCode::JUMP_IF_FALSE_POP);
    consume_(Token::LEFT_BRACE, "Expected '{' after condition.");
    nestedBlock_(false);
    // loop back up
    emitLoop_(loopStart);
    // escape the loop to here:
    setJumpDestination_(jumpToEnd);
}

void Compiler::synchronise_() {
    // don't stop panicking if we have had a fatal error: 
    if( hadFatalError_ ) return;

    // try and find a boundary which seems like a good sync point
    panicMode_ = false;
    while( currentToken_.type != Token::END ){
        // stop if the previous token looks like the end of a declaration/statement:
        if( previousToken_.type == Token::SEMICOLON ) return;

        // the following tokens look like the start of a new declaration/statement:
        switch( currentToken_.type ){
            case Token::CONST:
            case Token::FN:
            case Token::VAR:
            case Token::FOR:
            case Token::IF:
            case Token::WHILE:
            case Token::PRINT:
            case Token::RETURN:
                return;

            default: break;  // keep spinning
        }
        advance_();
    }
}

void Compiler::beginScope_() {
    currentEnv_->scopeDepth++;
}

void Compiler::endScope_() {
    currentEnv_->scopeDepth--;

    // At the end of a scope, remove all local variables from the value stack
    uint8_t n = currentEnv_->freeLocals();
    if( n > 0 ) emitBytes_(OpCode::POP_N, n);
}

void Compiler::expressionBlock_() {
    bool isExpression = block_(true);
    if( !isExpression ){
        errorAtPrevious_("Expression block must end in an expression.");
    }
}

bool Compiler::block_(bool canBeExpression) {
    // parse declarations (and statements) until hit the closing brace
    bool isExpression = false;
    while( !check_(Token::RIGHT_BRACE) && !check_(Token::END) ){
        if( isExpression ){
            errorAtPrevious_("Expression only allowed at end of block.");
        }
        isExpression = declaration_(canBeExpression);
    }
    consume_(Token::RIGHT_BRACE, "Expected '}' after block.");

    return isExpression;
}

bool Compiler::nestedBlock_(bool canBeExpression) {
    beginScope_();
    bool isExpression = block_(canBeExpression);
    endScope_();
    return isExpression;
}

void Compiler::parse_(Precedence precedence) {
    // Next token
    advance_();

    // Perform prefix rule of the token first:
    auto prefixRule = getRule_(previousToken_.type)->prefix;
    if( prefixRule == NULL ){
        errorAtPrevious_("Expected expression");
        return;
    }
    // Check whether assignment is possible and pass down to the rule (if it cares)
    bool canAssign = precedence <= Precedence::ASSIGNMENT;
    prefixRule(canAssign);

    // Perforce infix rules on tokens from left to right:
    for( ;; ){
        ParseRule const * rule = getRule_(currentToken_.type);
        if( rule->precedence < precedence ){
            // Stop: the new token has lower precedence so is not part of the current operand
            break;
        }
        // Consume and then compile the operator:
        advance_();
        rule->infix(canAssign);  // Can't be NULL as Precedence > NONE (refer getRule_ table)
    }
    // handle a case where assignment is badly placed, otherwise this isn't handled!
    if( canAssign && match_(Token::EQUAL) ){
        errorAtPrevious_("Invalid assignment target.");
    }
}

uint8_t Compiler::makeIdentifierLiteral_(Token & name) {
    return makeLiteral_(Value::string(
        ObjString::newString(vm_, name.start, name.length)
    ));
}

int Compiler::emitJump_(uint8_t instr) {
    emitByte_(instr);
    // placeholder value:
    emitByte_(0xFF);
    emitByte_(0xFF);
    // location of placeholder
    return getCurrentChunk_()->count() - 2;
}

void Compiler::setJumpDestination_(int offset) {
    Chunk * chunk = getCurrentChunk_();

    // how far to jump:
    int jumpLen = chunk->count() - offset - 2;
    if( jumpLen > UINT16_MAX ){
        errorAtPrevious_("Too much code to jump over.");
    }
    // set value:
    chunk->getCode()[offset] = (uint8_t)(jumpLen >> 8);
    chunk->getCode()[offset+1] = (uint8_t)(jumpLen & 0xFF);
}


void Compiler::emitLoop_(int loopStart) {
    emitByte_(OpCode::LOOP);
    int offset = getCurrentChunk_()->count() - loopStart + 2;
    if( offset > UINT16_MAX ) errorAtPrevious_("Loop body is too large.");
    emitByte_((uint8_t)((offset >> 8) & 0xff));
    emitByte_((uint8_t)(offset & 0xff));
}

void Compiler::grouping_() {
    // The opening '( is already consumed, expect an expression next:
    expression_();

    // consume the closing brace:
    consume_(Token::RIGHT_PAREN, "Expected ')' after expression");
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

void Compiler::call_() {
    // parse arguments:
    uint8_t argCount = 0;
    if( !check_(Token::RIGHT_PAREN) ) {
        do {
            expression_();
            if( argCount == 255 ) {
                errorAtPrevious_("Can't have more than 255 arguments.");
            }
            argCount ++;
        } while( match_(Token::COMMA) );
    }
    consume_(Token::RIGHT_PAREN, "Expected ')' after arguments.");

    emitBytes_(OpCode::CALL, argCount);
}

void Compiler::list_() {
    uint8_t numEntries = 0;
    if( !check_(Token::RIGHT_BRACKET) ) {
        do {
            expression_();
            if( numEntries == 255 ){
                errorAtPrevious_("Can't have more than 255 elements in list initialiser.");
            }
            numEntries ++;
        } while( match_(Token::COMMA) );
    }
    consume_(Token::RIGHT_BRACKET, "Expected ']' after list elements.");

    emitBytes_(OpCode::MAKE_LIST, numEntries);
}

void Compiler::type_() {
    consume_(Token::LEFT_PAREN, "Expected '(' after 'type'.");
    // type built-in takes a single value:
    expression_();
    consume_(Token::RIGHT_PAREN, "Expected ')' after argument.");
    emitByte_(OpCode::TYPE);
}

void Compiler::print_() {
    consume_(Token::LEFT_PAREN, "Expected '(' after 'print'.");
    // print built-in takes a single value:
    expression_();
    consume_(Token::RIGHT_PAREN, "Expected ')' after argument.");
    emitByte_(OpCode::PRINT);
}

void Compiler::index_() {
    expression_();
    consume_(Token::RIGHT_BRACKET, "Expected ']' after index.");
    emitByte_(OpCode::INDEX_GET);
}

void Compiler::number_() {
    // shouldn't fail as we already validated the token as a number:
    double n = strtod(previousToken_.start, nullptr);
    emitLiteral_(Value::number(n));
}

void Compiler::string_() {
    ObjString * str = ObjString::newString(vm_, previousToken_.start+1, previousToken_.length-2);
    emitLiteral_(Value::string(str));
}

void Compiler::variable_(bool canAssign) {
    getSetVariable_(previousToken_, canAssign);
}

void Compiler::getSetVariable_(Token & name, bool canAssign) {
    uint8_t getOp, setOp, arg; // opcodes for getting and setting the variable, and their argument

    // first, try to look up
    bool isConst;
    int res = currentEnv_->resolveLocal(this, name, isConst);
    if( res != Local::NOT_FOUND ){
        // its a local variable
        getOp = OpCode::GET_LOCAL;
        setOp = OpCode::SET_LOCAL;
        arg = (uint8_t)res;  // arg is the stack position of the local var

    }else if((res = currentEnv_->resolveUpvalue(this, name, isConst)) != Local::NOT_FOUND) {
        // its an upvalue
        getOp = OpCode::GET_UPVALUE;
        setOp = OpCode::SET_UPVALUE;
        arg = (uint8_t)res;  // stack position of upvalue

    }else{
        // its a global variable
        isConst = false; // assume not constant - checked at runtime
        getOp = OpCode::GET_GLOBAL;
        setOp = OpCode::SET_GLOBAL;
        arg = makeIdentifierLiteral_(name);  // arg is the literal index of the globals name
    }

    // identify whether we are setting or getting a variable:
    if( canAssign && match_(Token::EQUAL) ){
        if( isConst ){
            errorAtPrevious_("Cannot redefine a const variable.");
        }
        // setting the variable:
        expression_();  // the value to set
        emitBytes_(setOp, arg);
    }else{
        // getting the variable:
        emitBytes_(getOp, arg);
    }
}

// Macros to define lambdas to call each function with or without parameter `canAssign`
#define ASSIGNMENT_RULE(fn) [this](bool canAssign){ this->fn(canAssign); }
#define RULE(fn) [this](bool canAssign){ (void) canAssign; this->fn(); }

// TODO split into infix and prefix rule:
ParseRule const * Compiler::getRule_(Token::Type type) {
    static const ParseRule rules[] = {
        // token type             prefix func                 infix func     infix precedence
        [Token::LEFT_PAREN]    = {RULE(grouping_),            RULE(call_),   Precedence::CALL},
        [Token::RIGHT_PAREN]   = {NULL,                       NULL,          Precedence::NONE},
        [Token::LEFT_BRACE]    = {RULE(expressionBlock_),     NULL,          Precedence::NONE},
        [Token::RIGHT_BRACE]   = {NULL,                       NULL,          Precedence::NONE},
        [Token::LEFT_BRACKET]  = {RULE(list_),                RULE(index_),  Precedence::CALL},
        [Token::RIGHT_BRACKET] = {NULL,                       NULL,          Precedence::NONE},
        [Token::COMMA]         = {NULL,                       NULL,          Precedence::NONE},
        [Token::MINUS]         = {RULE(unary_),               RULE(binary_), Precedence::TERM},
        [Token::PLUS]          = {NULL,                       RULE(binary_), Precedence::TERM},
        [Token::SEMICOLON]     = {NULL,                       NULL,          Precedence::NONE},
        [Token::SLASH]         = {NULL,                       RULE(binary_), Precedence::FACTOR},
        [Token::STAR]          = {NULL,                       RULE(binary_), Precedence::FACTOR},
        [Token::BANG]          = {RULE(unary_),               NULL,          Precedence::NONE},
        [Token::BANG_EQUAL]    = {NULL,                       RULE(binary_), Precedence::EQUALITY},
        [Token::EQUAL]         = {NULL,                       NULL,          Precedence::NONE},
        [Token::EQUAL_EQUAL]   = {NULL,                       RULE(binary_), Precedence::EQUALITY},
        [Token::GREATER]       = {NULL,                       RULE(binary_), Precedence::COMPARISON},
        [Token::GREATER_EQUAL] = {NULL,                       RULE(binary_), Precedence::COMPARISON},
        [Token::LESS]          = {NULL,                       RULE(binary_), Precedence::COMPARISON},
        [Token::LESS_EQUAL]    = {NULL,                       RULE(binary_), Precedence::COMPARISON},
        [Token::IDENTIFIER]    = {ASSIGNMENT_RULE(variable_), NULL,          Precedence::NONE},
        [Token::STRING]        = {RULE(string_),              NULL,          Precedence::NONE},
        [Token::NUMBER]        = {RULE(number_),              NULL,          Precedence::NONE},
        [Token::AND]           = {NULL,                       RULE(and_),    Precedence::NONE},
        [Token::BOOL]          = {RULE(emitBoolType_),        NULL,          Precedence::NONE},
        [Token::CONST]         = {NULL,                       NULL,          Precedence::NONE},
        [Token::ELIF]          = {NULL,                       NULL,          Precedence::NONE},
        [Token::ELSE]          = {NULL,                       NULL,          Precedence::NONE},
        [Token::FALSE]         = {RULE(emitFalse_),           NULL,          Precedence::NONE},
        [Token::FOR]           = {NULL,                       NULL,          Precedence::NONE},
        [Token::FN]            = {RULE(funcAnonymous_),       NULL,          Precedence::NONE},
        [Token::FLOAT]         = {RULE(emitFloatType_),       NULL,          Precedence::NONE},
        [Token::IF]            = {RULE(ifExpression_),        NULL,          Precedence::NONE},
        [Token::NIL]           = {RULE(emitNil_),             NULL,          Precedence::NONE},
        [Token::OR]            = {NULL,                       RULE(or_),     Precedence::NONE},
        [Token::OBJECT]        = {RULE(emitObjectType_),      NULL,          Precedence::NONE},
        [Token::PRINT]         = {RULE(print_),               NULL,          Precedence::NONE},
        [Token::RETURN]        = {NULL,                       NULL,          Precedence::NONE},
        [Token::STRING_TYPE]   = {RULE(emitStringType_),      NULL,          Precedence::NONE},
        [Token::TRUE]          = {RULE(emitTrue_),            NULL,          Precedence::NONE},
        [Token::TYPE]          = {RULE(type_),                NULL,          Precedence::NONE},
        [Token::TYPEID]        = {RULE(emitTypeIdType_),      NULL,          Precedence::NONE},
        [Token::VAR]           = {NULL,                       NULL,          Precedence::NONE},
        [Token::WHILE]         = {NULL,                       NULL,          Precedence::NONE},
        [Token::ERROR]         = {NULL,                       NULL,          Precedence::NONE},
        [Token::END]           = {NULL,                       NULL,          Precedence::NONE},
    };
    return &rules[type];
}
#undef RULE

void Compiler::fatalError_(const char* fmt, ...) {
    hadFatalError_ = true;

    va_list args;
    va_start(args, fmt);
    errorAtVargs_(&currentToken_, fmt, args);
    va_end(args);
}

void Compiler::errorAtCurrent_(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    errorAtVargs_(&currentToken_, fmt, args);
    va_end(args);
}

void Compiler::errorAtPrevious_(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    errorAtVargs_(&previousToken_, fmt, args);
    va_end(args);
}

void Compiler::errorAt_(Token* token, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    errorAtVargs_(token, fmt, args);
    va_end(args);
}

void Compiler::errorAtVargs_(Token* token, const char* fmt, va_list args) {
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

    fprintf(stderr, ": ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    hadError_ = true;
}

