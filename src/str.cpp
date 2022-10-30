
#include "str.hpp"
#include <string.h>

static uint32_t calcHash_(char const * str, int length);

StringView::StringView(char const * c) {
    chars_ = c;
    length_ = (int)strlen(chars_);  // TODO does this include null terminator? Should it?
    hash_ = calcHash_(chars_, length_);
}

StringView::StringView(char const * c, int len) {
    chars_ = c;
    length_ = len;
    hash_ = calcHash_(chars_, length_);
}

/**
 * ObjString
*/
ObjString::ObjString(Vm * vm, char const * str, int length): Obj(vm, Obj::Type::STRING) {
    // copy string to buffer:
    chars_ = new char[length + 1];
    memcpy(chars_, str, length);
    chars_[length] = '\0';
    length_ = length;
    hash_ = calcHash_(str, length);
}

ObjString::~ObjString() {
    delete[] chars_;
}

/**
 * Hash value of a String
 */
struct StringHash {
    std::size_t operator()(String const * key) const {
        return key->getHash();
    }
};

/**
 * Compare two Strings
 */
struct StringEqual {
    bool operator()(String const * lhs, String const * rhs) const {
        return (lhs->getLength() == rhs->getLength()) && 
               (strcmp(lhs->get(), rhs->get()) == 0);
    }
};

/**
 * Hidden helper class
*/
class InternedStringHashSet : public std::unordered_set<String*, StringHash, StringEqual> {
public:
    InternedStringHashSet() {}
    virtual ~InternedStringHashSet() {}
};

/**
 * InternedStringSet class def:
*/

InternedStringSet::InternedStringSet() {
    hashSet_ = new InternedStringHashSet();
}

InternedStringSet::~InternedStringSet() {
    delete hashSet_;
}

ObjString * InternedStringSet::add(Vm * vm, char const * chars, int len) {
    // Search if string is already interned:
    StringView lookup(chars);
    auto key = hashSet_->find(&lookup);
    if( key == hashSet_->end() ){
        // Not found: make a new object:
        ObjString * ostr = new ObjString(vm, chars, len);
        hashSet_->emplace(ostr);
    }
    // Found it:
    // Must be an ObjString because thats all we ever add to the set
    return (ObjString*) *key;
}

static uint32_t calcHash_(char const * str, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)str[i];
        hash *= 16777619;
    }
    return hash;
}

void InternedStringSet::debug() {
    printf("Interned string set:\n");
    for( auto & it : *hashSet_ ){
        printf("  %p: '%s'\n", (ObjString*) it, it->get());
    }
}
