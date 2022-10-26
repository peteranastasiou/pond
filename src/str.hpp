
#pragma once

#include "object.hpp"
#include <unordered_set>

/**
 * Interface for string keys
 */
struct Key {
    virtual std::string const & get() const = 0;
};

/**
 * Garbage Collected String Object
 */
struct ObjString : public Obj, Key {
    ObjString(Vm * vm) : Obj(vm) {}
    ObjString(Vm * vm, std::string s) : Obj(vm), str(s) {}
    virtual ~ObjString() {}  // std::string is automatically deleted

    virtual std::string toString() override { return str; }

    virtual std::string const & get() const override { return str; }

    std::string str;
};

/**
 * Throw-away object used for string lookups
 */
struct Lookup : public Key {
    Lookup(std::string s) : str(s) {}
    virtual ~Lookup() {}

    virtual std::string const & get() const override { return str; }

    std::string str;
};

/**
 * Hash value of a Key
 */
struct KeyHash {
    std::size_t operator()(Key const * key) const {
        return std::hash<std::string>()(key->get());
    }
};

/**
 * Compare two Keys
 */
struct KeysEqual {
    bool operator()(Key const * lhs, Key const * rhs) const {
        return !lhs->get().compare(rhs->get());
    }
};

/**
 * Set of strings. All elements must be ObjStrings
 * Key interface is used so we can do cheap lookups
 */
typedef std::unordered_set<Key*, KeyHash, KeysEqual> InternedStringSet;

