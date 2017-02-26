#include <stdbool.h>
#include <stdlib.h>
#include "lock.h"

typedef struct TASLock {
    volatile bool locked;
} TASLock;

void tas_lock(struct TASLock* this) {
    while (true) {
        while (this->locked) {};
        if (!__sync_lock_test_and_set(&this->locked, true)) {
            return;
        }
    }
}

void tas_unlock(struct TASLock* this) {
    this->locked = false;
}

lock_iface new_taslock() {
    TASLock* inner_lock = malloc(sizeof(TASLock));
    inner_lock->locked = false;
    lock_iface ret = {.ptr = inner_lock, ._lock = tas_lock, ._unlock = tas_unlock};
    return ret;
}

