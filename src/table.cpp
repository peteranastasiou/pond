
#include "table.hpp"

// ----------------------------------------------------------------------------
// String Comparison Helpers
// ----------------------------------------------------------------------------
std::size_t StringHash::operator()(String const * key) const {
    return key->getHash();
}

bool StringEqual::operator()(String const * lhs, String const * rhs) const {
    return (lhs->getLength() == rhs->getLength()) && 
            (memcmp(lhs->get(), rhs->get(), lhs->getLength()) == 0);
}

// ----------------------------------------------------------------------------
// InternedStringSet
// ----------------------------------------------------------------------------
StringSet::StringSet() {
}

StringSet::~StringSet() {
}

ObjString * StringSet::find(char const * chars, int len) {
    // Search if string is already interned:
    StringView lookup(chars, len);
    auto key = set_.find(&lookup);
    if( key == set_.end() )  return nullptr;  // not found
    // Found it:
    // Must be an ObjString because thats all we ever add to the set
    return (ObjString*) *key;
}

void StringSet::add(ObjString * ostr) {
    set_.emplace(ostr);
}

void StringSet::debug() {
    printf("Interned string set:\n");
    for( auto & it : set_ ){
        printf("  %p: 0x%8x %3i '%s'\n", 
               (ObjString*) it, it->getHash(), it->getLength(), it->get());
    }
}

// ----------------------------------------------------------------------------
// HashMap
// ----------------------------------------------------------------------------


HashMap::HashMap() {
}

HashMap::~HashMap() {
}

bool HashMap::set(ObjString * key, Value value) {
    // returns pair of iterator and bool isNew:
    return map_.insert_or_assign(key, value).second;
}

bool HashMap::get(ObjString * key, Value & value) {
    auto search = map_.find(key);
    if( search == map_.end() ) return false;
    value = (*search).second;
    return true;
}

bool HashMap::remove(ObjString * key) {
    return map_.erase(key) == 1;
}

void HashMap::debug() {
    for( const auto & [key, value] : map_ ){
        printf("  '%s': ", key->get());
        value.print();
        printf("\n");
    }
}
