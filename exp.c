//
// Created by Zixiong Liu on 2/12/17.
//

#include "exp.h"
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include "lock.h"

#define INIT_DELAY 10
#define LIMIT 1000000
static __thread uint32_t delay_perturb = 0;
static __thread uint32_t delay = INIT_DELAY;
inline static void exp_backoff();


typedef struct ExpLock {
    volatile bool locked;
} ExpLock;

void exp_lock(struct ExpLock* this) {
    while (true) {
        while (this->locked) {};
        if (!__sync_lock_test_and_set(&this->locked, true)) {
            delay = INIT_DELAY;
            return;
        } else {
            exp_backoff();
        }
    }
}

inline static void exp_backoff() {
    if (delay_perturb == 0) {
        delay_perturb = (uint32_t) (((double)random() / (double)RAND_MAX) * 10);
    }
    usleep((useconds_t)delay + delay_perturb);
    delay = delay + delay_perturb;
    if (delay > LIMIT) delay = INIT_DELAY;
}

void exp_unlock(struct ExpLock* this) {
    this->locked = false;
}

lock_iface new_explock() {
    ExpLock* inner_lock = malloc(sizeof(ExpLock));
    inner_lock->locked = false;
    lock_iface ret = {.ptr = inner_lock, ._lock = exp_lock, ._unlock = exp_unlock};
    srand(time(0));
    return ret;
}

