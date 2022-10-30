#pragma once

#include <string>

class Vm;
class Str;

/**
 * NOTE: if objects are all created via Vm, then we can do the register/deregister there
 * This may mean we don't need a custom "Lookup" class for strings, as they will be cheap objects again
*/

struct Obj {
    enum Type {
        STRING
    };

    Obj(Vm * vm, Type t);

    virtual ~Obj();

    // virtual void toString(Str & str)=0;

    Type type;
    Obj * next;  // linked list of all objects

private:
    Vm * vm_;
};
