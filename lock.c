//
// Created by Zixiong Liu on 2/12/17.
//

#include <stddef.h>
#include "lock.h"

void lock(lock_iface * this) {
    if (this->ptr  != NULL)
        this->_lock(this->ptr);
}

void unlock(lock_iface * this) {
    if (this->ptr != NULL)
        this->_unlock(this->ptr);
}