#pragma once

#include "object.hpp"
#include "str.hpp"
#include <string>
#include <string.h>

struct Value {
    enum Type {
        NIL,
        BOOL,
        NUMBER,
        OBJECT
    } type;

    union {
        bool boolean;
        double number;
        Obj * obj;
    } as;

    // Constructor-likes:
    static inline Value nil() { return (Value){NIL, {.number = 0}}; }
    static inline Value boolean(bool b) { return (Value){BOOL, {.boolean = b}}; }
    static inline Value number(double n) { return (Value){NUMBER, {.number = n}}; }
    static inline Value object(Obj * o) { return (Value){OBJECT, {.obj = o}}; }

    // Helpers for value types
    inline bool isNil() const { return type == NIL; }
    inline bool isBoolean() const { return type == BOOL; }
    inline bool isNumber() const { return type == NUMBER; }
    inline bool isObject() const { return type == OBJECT; }

    // Helpers for object types
    inline bool isObjType(Obj::Type t) const { return isObject() && as.obj->type == t; }
    inline bool isString() const { return isObjType(Obj::Type::STRING); }
    inline ObjString * asObjString() const { return (ObjString*)as.obj; }
    inline char const * asCString() const { return asObjString()->get(); }

    // value methods
    bool equals(Value other) const;
    ObjString * toString(Vm * vm);
    void print() const;
};
