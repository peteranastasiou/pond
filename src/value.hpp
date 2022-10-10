#pragma once

struct Value {
    enum Type {
        NIL,
        BOOL,
        NUMBER    // TODO split into int and double
    } type;
    union {
        bool boolean;
        double number;
    } as;

    static inline Value nil() { return (Value){NIL, {.number = 0}}; }
    static inline Value boolean(bool b) { return (Value){BOOL, {.boolean = b}}; }
    static inline Value number(double n) { return (Value){NUMBER, {.number = n}}; }

    inline bool isNil() { return type == NIL; }
    inline bool isBoolean() { return type == BOOL; }
    inline bool isNumber() { return type == NUMBER; }

    bool equals(Value other);
    void print();
};
