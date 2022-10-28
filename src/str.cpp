
#include "str.hpp"

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
 * Hidden helper class
*/
class InternedStringHashSet : public std::unordered_set<Key*, KeyHash, KeysEqual> {
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

ObjString * InternedStringSet::find(std::string s) const {
    // Search if string is already interned:
    Lookup lookup{s};
    auto key = hashSet_->find(&lookup);
    if( key == hashSet_->end() ){
        return nullptr; // not found
    }
    // Found it:
    // Must be an ObjString because thats all we ever add to the set
    return (ObjString*) *key;
}

ObjString * InternedStringSet::add(Vm * vm, std::string s) {
    ObjString * ostr = new ObjString(vm, s);
    hashSet_->emplace(ostr);
    return ostr;
}

void InternedStringSet::debug() {
    printf("Interned string set:\n");
    for( auto & it : *hashSet_ ){
        printf("  %p: '%s'\n", (ObjString*) it, it->get().c_str());
    }
}
