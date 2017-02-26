//
// Created by Zixiong Liu on 2/12/17.
//

#include "lock.h"

void lock(lock_iface * this) {
    this->_lock(this->ptr);
}

void unlock(lock_iface * this) {
    this->_unlock(this->ptr);
}