#pragma once

#include "object.hpp"
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
    static inline Value string(char const * c, int l) { return object(new ObjString(c, l)); }
    static inline Value string(char const * c) { return object(new ObjString(c, (int)strlen(c))); }
    static inline Value string(std::string s) { return object(new ObjString(s)); }

    // Helpers for value types
    inline bool isNil() { return type == NIL; }
    inline bool isBoolean() { return type == BOOL; }
    inline bool isNumber() { return type == NUMBER; }
    inline bool isObject() { return type == OBJECT; }

    // Helpers for object types
    inline bool isObjType(Obj::Type t) { return isObject() && as.obj->type == t; }
    inline bool isString() { return isObjType(Obj::Type::STRING); }
    inline ObjString * asObjString() { return (ObjString*)as.obj; }
    inline std::string asString() { return asObjString()->str; }
    inline char const * asCString() { return asString().c_str(); }

    // value methods
    bool equals(Value other);
    std::string toString();
    void print();
};
