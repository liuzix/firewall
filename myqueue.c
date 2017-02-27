//
// Created by Zixiong Liu on 1/29/17.
//

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <sched.h>
#include <pthread.h>
#include "myqueue.h"
#include "Utils/packetsource.h"
void* test_queue_dispatcher(void* ptr);
bool is_empty(struct queue *q) {
    return (q->tail == q->head);
}

// It should only be called by the enqueuing thread
// to ensure correctness
bool is_full(struct queue* q) {
    return (q->tail - q->head == q->len);
}
extern lock_iface (*lock_gen)(void);
enum strategy_t {LOCK_FREE, HOMEQUEUE, RANDOMQUEUE, AWESOME};
extern enum strategy_t strat;
void queue_init(struct queue *q, size_t len) {
    q->tail = 0;
    q->head = 0;
    q->buf = calloc(len, sizeof(Packet_t));
    q->len = len;
    if (lock_gen != NULL) {
        q->lock_inst = lock_gen();
    } else {
        q->lock_inst.ptr = NULL;
    }
    assert(q->buf != NULL);
}


void queue_enqueue(struct queue *q, Packet_t p) {
    assert(q != NULL);
    while (is_full(q)) {
        if (strat != LOCK_FREE)
            unlock(&q->lock_inst);
        sched_yield();
        if (strat != LOCK_FREE)
            lock(&q->lock_inst);
    }
    q->buf[q->tail % q->len] = p;
    q->tail ++;
}

Packet_t queue_dequeue(struct queue *q) {
    assert(q != NULL);
    while (is_empty(q)) {
        if (strat != LOCK_FREE)
            unlock(&q->lock_inst);
        sched_yield();
        if (strat != LOCK_FREE)
            lock(&q->lock_inst);
    }
    Packet_t r = q->buf[q->head % q->len];
    q->head ++;
    return r;
}

void test_queue() {
    struct queue Q;
    queue_init(&Q, 10);
    pthread_t t;
    pthread_create(&t, NULL, test_queue_dispatcher, &Q);
    for (int i = 0; i < 100; i++) {
        Packet_t p = queue_dequeue(&Q);
        assert(p.seed == i);
        //printf("%d\n", p.seed);
    }
    pthread_join(t, NULL);
    printf("test_queue passed.");

}

void* test_queue_dispatcher(void* ptr) {
    struct queue *Q = ptr;
    for (int i = 0; i < 100; i++) {
        Packet_t test_p;
        test_p.seed = i;
        queue_enqueue(Q, test_p);
    }
    return NULL;
}