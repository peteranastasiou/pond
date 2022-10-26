
#include "value.hpp"
#include "util.hpp"

#include <stdio.h>

bool Value::equals(Value other) {
    printf("Equals: value type:%i %i\n", type, other.type);
    if( type != other.type ) return false;

    switch( type ){
        case NIL:     return true;
        case BOOL:    return as.boolean == other.as.boolean;
        case NUMBER:  return as.number == as.number;
        case OBJECT:{
            printf("  Object type:%i\n", as.obj->type);
            if( as.obj->type == Obj::Type::STRING ){
                printf(" check %p == %p\n", asObjString(), other.asObjString());
                // all strings are interned --> therefore can compare pointers
                return asObjString() == other.asObjString();
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
