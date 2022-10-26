
#include "object.hpp"
#include "vm.hpp"


Obj::Obj(Vm * vm, Type t): type(t), vm_(vm) {
    vm_->registerObj(this);
}

Obj::~Obj(){
    vm_->deregisterObj(this);
}
