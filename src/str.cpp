
#include "str.hpp"
#include "vm.hpp"
#include <string.h>
#include <stdarg.h>

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
ObjString * ObjString::newString(Vm * vm, char const * str) {
    return newString(vm, str, (int)strlen(str));
}

ObjString * ObjString::newString(Vm * vm, char const * str, int length) {
    // is string already interned?
    printf("!Find string: '%.*s'\n", length, str);
    ObjString * ostr = vm->getInternedStrings()->find(str, length);
    if( ostr != nullptr ) return ostr;  // already have that one!

    // Allocate space for new string
    char * chars = new char [length + 1];
    memcpy(chars, str, length);
    chars[length] = '\0';  // ensure null terminated

    // make a new string
    return new ObjString(vm, chars, length);
}

ObjString * ObjString::newStringFmt(Vm * vm, const char* fmt, ...) {
    va_list args;

    // Work out buffer size by doing a dry run: 
    va_start(args, fmt);
    int len = vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    // Now do the real thing:
    char * chars = new char[len+1];
    va_start(args, fmt);
    vsnprintf(chars, len+1, fmt, args);
    va_end(args);

    // is string already interned?
    printf("!Find string: '%s'\n", chars);
    ObjString * ostr = vm->getInternedStrings()->find(chars, len);
    if( ostr != nullptr ) return ostr;  // already have that one!

    // make a new string
    return new ObjString(vm, chars, len);
}

ObjString * ObjString::concatenate(Vm * vm, ObjString * a, ObjString * b) {
    // Make a new character array combining the strings
    int aLen = a->getLength();
    int bLen = b->getLength();
    int len = aLen + bLen;
    char * chars = new char[len+1];
    memcpy(chars, a->get(), aLen);
    memcpy(&chars[aLen], b->get(), bLen);
    chars[len] = '\0';

    // is string already interned?
    printf("!Find string: '%s'\n", chars);
    ObjString * ostr = vm->getInternedStrings()->find(chars, len);
    if( ostr != nullptr ) return ostr;  // already have that one!

    // make a new string
    return new ObjString(vm, chars, len);
}

ObjString::ObjString(Vm * vm, char const * chars, int length): Obj(vm, Obj::Type::STRING)  {
    chars_ = chars;
    length_ = length;
    hash_ = calcHash_(chars_, length_);

    printf("!Adding a new string: '%s', %i, %u\n", chars_, length_, hash_);
    // Add to interned set
    vm->getInternedStrings()->add(this);
    printf("!Made new string\n");
}

ObjString::~ObjString() {
    delete[] chars_;
}

/**
 * Hash value of a String
 */
struct StringHash {
    std::size_t operator()(String const * key) const {
        printf("Get hash for %p [%i]'%.*s': %u\n",
            key, key->type(), key->getLength(), key->get(), key->getHash());
        return key->getHash();
    }
};

/**
 * Compare two Strings
 */
struct StringEqual {
    bool operator()(String const * lhs, String const * rhs) const {
        printf("Compare strings: [%i]'%.*s'==[%i]'%.*s', %i==%i, %i\n", 
               lhs->type(), lhs->getLength(), lhs->get(), 
               rhs->type(), rhs->getLength(), rhs->get(),
               lhs->getLength(), rhs->getLength(), 
               memcmp(lhs->get(), rhs->get(), lhs->getLength()));

        return (lhs->getLength() == rhs->getLength()) && 
               (memcmp(lhs->get(), rhs->get(), lhs->getLength()) == 0);
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

ObjString * InternedStringSet::find(char const * chars, int len) {
    // Search if string is already interned:
    StringView lookup(chars, len);
    auto key = hashSet_->find(&lookup);
    if( key == hashSet_->end() )  return nullptr;  // not found
    // Found it:
    // Must be an ObjString because thats all we ever add to the set
    return (ObjString*) *key;
}

void InternedStringSet::add(ObjString * ostr) {
    hashSet_->emplace(ostr);
}

void InternedStringSet::debug() {
    printf("Interned string set:\n");
    for( auto & it : *hashSet_ ){
        printf("  %p: '%s'\n", (ObjString*) it, it->get());
    }
}

static uint32_t calcHash_(char const * str, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)str[i];
        hash *= 16777619;
    }
    return hash;
}
