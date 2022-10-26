#pragma once

#include <string>

class Vm;

/**
 * NOTE: if objects are all created via Vm, then we can do the register/deregister there
 * This may mean we don't need a custom "Lookup" class for strings, as they will be cheap objects again
*/

struct Obj {
    Obj(Vm * vm);
    virtual ~Obj();

    virtual std::string toString()=0;
    
    enum Type {
        STRING
    } type;

    Obj * next;  // linked list of all objects

private:
    Vm * vm_;
};
