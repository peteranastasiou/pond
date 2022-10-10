
#include "value.hpp"

#include <stdio.h>


void printValue(Value value) {
  switch( value.type ){
    case Value::NIL:    printf("nil"); break;
    case Value::BOOL:   printf(value.as.boolean ? "true" : "false"); break;
    case Value::NUMBER: printf("%g", value.as.number); break;
  }
}
