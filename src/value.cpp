
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
        case NIL:     return ObjString::newString(vm, "nil");
        case BOOL:    return ObjString::newString(vm, as.boolean ? "true" : "false");
        case NUMBER:  return ObjString::newStringFmt(vm, "%g", as.number);
        case OBJECT:  return as.obj->toString();
        default:      return ObjString::newString(vm, "???");
    }
}

void Value::print() {
    switch( type ){
        case NIL:     printf("nil"); return;
        case BOOL:    printf(as.boolean ? "true" : "false"); return;
        case NUMBER:  printf("%g", as.number); return;
        case OBJECT:  as.obj->print(); return;
        default:      printf("???");
    }
}
