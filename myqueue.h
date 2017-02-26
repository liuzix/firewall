//
// Created by Zixiong Liu on 1/29/17.
//

#ifndef FIREWALL_MYQUEUE_H
#define FIREWALL_MYQUEUE_H

#include "Utils/packetsource.h"

struct queue {
    volatile size_t head;
    volatile size_t tail;
    size_t len;
    Packet_t *buf;
};

/******
 * Initialize a queue. Queue is initialized to be empty
 * @param q An queue object to be initialized
 * @param len The capacity of the queue
 */
void queue_init(struct queue* q, size_t len);

/*
 * Enqueue p into q. Block when full
 */
void queue_enqueue(struct queue* q, Packet_t p);

/*
 * Dequeue from q. Block when empty
 */
Packet_t queue_dequeue(struct queue *q);

void test_queue();

#endif //FIREWALL_MYQUEUE_H
