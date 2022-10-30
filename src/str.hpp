
#pragma once

#include "object.hpp"
#include <unordered_set>

/**
 * Interface for strings
 */
class String {
public:
    virtual ~String() {}
    virtual char const * get() const = 0;
    virtual uint32_t getHash() const = 0;
    virtual int getLength() const = 0;
};

/**
 * A StringView class which doesn't own its string
 */
class StringView: public String {
public:
    StringView(char const * c);
    StringView(char const * c, int len);

    virtual ~StringView() {}

    virtual char const * get() const override { return chars_; }
    virtual uint32_t getHash() const override { return hash_; }
    virtual int getLength() const override { return length_; }

private:
    char const * chars_;
    int length_;
    uint32_t hash_;
};

/**
 * Garbage-Collected String Object
*/
class ObjString : public Obj, public String {
public:
    /**
     * Copies string memory into this class
     * NOTE: takeString not required! (only used in lox in concatenate)
     */
    ObjString(Vm * vm, char const * str);
    ObjString(Vm * vm, char const * str, int length);

    virtual ~ObjString();

    // implment Obj interface
    // virtual char * toString() override { return chars; }

    // implement Key interface:
    virtual char const * get() const override { return chars_; }
    virtual uint32_t getHash() const override { return hash_; }
    virtual int getLength() const override { return length_; }

private:
    char * chars_;  // null terminated sequence
    int length_;
    uint32_t hash_;
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

    ObjString * add(Vm * vm, char const * chars, int len);

    void debug();

private:
    // Pointer to implementation, to hidden helper class:
    InternedStringHashSet * hashSet_;
};
