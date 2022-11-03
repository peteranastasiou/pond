#pragma once

#include "str.hpp"
#include "value.hpp"

#include "string.h"
#include <unordered_set>
#include <unordered_map>

/**
 * Hash value of a String (for HashSet/HashMap impl)
 */
struct StringHash {
    std::size_t operator()(String const * key) const;
};

/**
 * Compare two Strings (for HashSet/HashMap impl)
 */
struct StringEqual {
    bool operator()(String const * lhs, String const * rhs) const;
};

/**
 * Set of strings. All elements must be ObjStrings
 * Key interface is used so we can do cheap lookups
 */
class StringSet {
public:
    StringSet();
    ~StringSet();

    ObjString * find(char const * chars, int len);
    void add(ObjString * ostr);

    void debug();

private:
    std::unordered_set<String*, StringHash, StringEqual> set_;
};

/**
 * HashMap of <ObjString*> keys, and <Value> values
 */
class HashMap {
public:
    HashMap();
    ~HashMap();

    /**
     * Set a value for the given key
     * @return true if the key is new
     */
    bool set(ObjString * key, Value value);

    /**
     * Look up a value for the given key
     * @return true if the key/value exists
     */
    bool get(ObjString * key, Value & value);

    /**
     * Remove entry
     * @return true if an entry was deleted
     */
    bool remove(ObjString * key);

    void debug();

private:
    std::unordered_map<String*, Value, StringHash, StringEqual> map_;
};
