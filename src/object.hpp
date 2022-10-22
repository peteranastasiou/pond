#pragma once

#include <string.h>
#include <string>

class Vm;

struct Obj {
    enum Type {
        STRING
    } type;

    Obj * next;  // linked list of all objects

    Obj(Vm * vm);
    virtual ~Obj();

    virtual std::string toString()=0;
    
private:
    Vm * vm_;    // 
};

struct ObjString : public Obj {
    std::string str;

    ObjString(Vm * vm) : Obj(vm) {}
    ObjString(Vm * vm, char const * chars, int len) : Obj(vm), str(chars, chars + len) {}
    ObjString(Vm * vm, std::string s) : Obj(vm), str(s) {}
    virtual ~ObjString() {}  // std::string is automatically deleted

    virtual std::string toString() override;
};
