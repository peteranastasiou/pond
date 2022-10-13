
#include "value.hpp"

#include <stdio.h>

bool Value::equals(Value other) {
    if( type != other.type ) return false;

    switch( type ){
        case NIL:     return true;
        case BOOL:    return as.boolean == other.as.boolean;
        case NUMBER:  return as.number == as.number;
        case OBJECT:{
            if( as.obj->type == Obj::Type::STRING ){
                return asString().compare(other.asCString()) == 0;
            }
            return false; // TODO other object types
        }
        default:      return false;   // Unreachable
    }
}


std::string Value::toString() {
    
}

void Value::print() {
    switch( type ){
        case Value::NIL:    printf("nil"); break;
        case Value::BOOL:   printf(as.boolean ? "true" : "false"); break;
        case Value::NUMBER: printf("%g", as.number); break;
        case Value::OBJECT: printObject_(); break;
    }
}

void Value::printObject_() {
    switch( as.obj->type ){
        case Obj::Type::STRING: printf("%s", asCString()); break;
    }
}
