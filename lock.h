
#ifndef LOCK_H
#define LOCK_H
/// the common lock interface
typedef struct lock {
	void* ptr;
	void(*_lock)(void*);
	void(*_unlock)(void*);
} lock_iface;

void lock(lock_iface*);
void unlock(lock_iface*);

///
lock_iface new_taslock();
lock_iface new_explock();

#endif