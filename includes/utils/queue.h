//
// Created by Yichen Xu on 2020/8/18.
//

#ifndef DNS_RELAY_QUEUE_H
#define DNS_RELAY_QUEUE_H

#include "stdlib.h"
#include "assert.h"

typedef struct {
    int *buf;
    size_t size;
    int full;
    size_t enq;
    size_t deq;
} queue_t;

queue_t *q_init(size_t size);

int q_full(queue_t *q);

int q_empty(queue_t *q);

void q_enq(queue_t *q, int val);

int q_deq(queue_t *q);

int q_in(queue_t *q, int val);

void q_free(queue_t *q);

#endif //DNS_RELAY_QUEUE_H
