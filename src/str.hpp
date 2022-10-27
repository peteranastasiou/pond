
#pragma once

#include "object.hpp"
#include <unordered_set>

/**
 * Interface for string keys
 */
struct Key {
    virtual std::string const & get() const = 0;
};

struct ObjString : public Obj, Key {
    ObjString(Vm * vm) : Obj(vm, Obj::Type::STRING) {}
    ObjString(Vm * vm, std::string s) : Obj(vm, Obj::Type::STRING), str(s) {}
    virtual ~ObjString() {}  // std::string is automatically deleted

    virtual std::string toString() override { return str; }

    virtual std::string const & get() const override { return str; }

    std::string str;
};

// predeclare Vm
class Vm;

// predeclare hidden helper class
class InternedStringHashSet;

/**
 * Set of strings. All elements must be ObjStrings
 * Key interface is used so we can do cheap lookups
 */
class InternedStringSet {
public:
    InternedStringSet();
    virtual ~InternedStringSet();

    ObjString * find(std::string s) const;  // TODO change to char const *

    ObjString * add(Vm * vm, std::string s);  // TODO change to char const *

private:
    // Pointer to implementation, to hidden helper class:
    InternedStringHashSet * hashSet_;
};
