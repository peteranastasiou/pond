
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
                // all strings are interned --> therefore can compare pointers
                return asObjString() == other.asObjString();
            }
            return false; // TODO other object types
        }
        default:      return false;   // Unreachable
    }
}

ObjString * Value::toString(Vm * vm) {
    switch( type ){
        case NIL:     return new ObjString(vm, "nil");
        case BOOL:    return as.boolean ? "true" : "false";
        case NUMBER:  return util::format("%g", as.number);
        case OBJECT:  return as.obj->toString();
        default:      return "???";
    }
}

void Value::print() {
    printf("%s", toString().c_str());
}
