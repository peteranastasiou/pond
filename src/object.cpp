
#include "object.hpp"
#include "vm.hpp"


Obj::Obj(Vm * vm): vm_(vm) {
    vm_->registerObj(this);
}

Obj::~Obj(){
    vm_->deregisterObj(this);
}
