
#include "value.hpp"

#include <stdio.h>


bool Value::equals(Value other) {
    if( type != other.type ) return false;

    switch( type ){
        case NIL:     return true;
        case BOOL:    return as.boolean == other.as.boolean;
        case NUMBER:  return as.number == as.number;
        default:      return false;   // Unreachable
    }
}

void Value::print() {
    switch( type ){
        case Value::NIL:    printf("nil"); break;
        case Value::BOOL:   printf(as.boolean ? "true" : "false"); break;
        case Value::NUMBER: printf("%g", as.number); break;
    }
}
