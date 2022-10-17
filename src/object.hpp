#pragma once

#include <string.h>
#include <string>

struct Obj {
    enum Type {
        STRING
    } type;

    Obj * next;  // linked list of all objects

    Obj();
    ~Obj();

    virtual std::string toString()=0;
};

struct ObjString : public Obj {
    std::string str;

    ObjString() {}
    ObjString(char const * chars, int len) : str(chars, chars + len) {}
    ObjString(std::string s) : str(s) {}
    ~ObjString() {}

    virtual std::string toString() override;
};
