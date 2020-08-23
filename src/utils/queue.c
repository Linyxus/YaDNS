//
// Created by Yichen Xu on 2020/8/18.
//

#include "utils/queue.h"

queue_t *q_init(size_t size) {
    queue_t *q = malloc(sizeof(queue_t));
    q->size = size;
    q->buf = malloc(sizeof(int) * size);
    q->full = 0;
    q->enq = 0;
    q->deq = 0;

    return q;
}

void q_free(queue_t *q) {
    free(q->buf);
    free(q);
}

int q_full(queue_t *q) {
    return q->full;
}

int q_empty(queue_t *q) {
    return (q->enq == q->deq) && !q->full;
}

void q_enq(queue_t *q, int val) {
    assert(!q_full(q));
    q->buf[q->enq] = val;
    q->enq = (q->enq + 1) % q->size;
    q->full = q->enq == q->deq;
}

int q_deq(queue_t *q) {
    assert(!q_empty(q));
    int ret = q->buf[q->deq];
    q->deq = (q->deq + 1) % q->size;
    q->full = 0;
    return ret;
}

int q_in(queue_t *q, int val) {
    if (q_empty(q)) return 0;
    int i = q->deq;
    do {
        if (q->buf[i] == val) return 1;
        i = (i + 1) % q->size;
    } while (i != q->enq);
    return 0;
}

