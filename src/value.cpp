
#include "value.hpp"
#include "util.hpp"

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
    switch( type ){
        case NIL:     return "nil";
        case BOOL:    return as.boolean ? "true" : "false";
        case NUMBER:  return util::format("%g", as.number);
        case OBJECT:  return as.obj->toString();
        default:      return "???";
    }
}

void Value::print() {
    printf("%s", toString().c_str());
}
