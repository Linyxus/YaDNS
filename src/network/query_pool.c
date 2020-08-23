//
// Created by Yichen Xu on 2020/8/18.
//

#include "network/query_pool.h"

query_pool_t *qpool_init(size_t pool_size) {
    query_pool_t *pool = malloc(sizeof(query_pool_t));
    pool->pool_size = pool_size;
    pool->pool = malloc(sizeof(query_t) * pool_size);
    pool->idx_queue = q_init(pool_size);
    for (int i = 0; i < pool_size; i++) {
        q_enq(pool->idx_queue, i);
    }

    return pool;
}

int qpool_full(query_pool_t *pool) {
    return q_empty(pool->idx_queue);
}

int qpool_insert(query_pool_t *pool, struct sockaddr addr, char *msg, size_t msg_len) {
    assert(!qpool_full(pool));
    size_t idx = q_deq(pool->idx_queue);
    pool->pool[idx].addr = addr;
    pool->pool[idx].msg = malloc(msg_len);
    memcpy(pool->pool[idx].msg, msg, msg_len);
    pool->pool[idx].msg_len = msg_len;
    pool->pool[idx].dns_id = get_dns_id(msg);
    return idx;
}

void qpool_remove(query_pool_t *pool, size_t idx) {
    assert(!q_in(pool->idx_queue, idx));
    if (pool->pool[idx].msg) {
        free(pool->pool[idx].msg);
    }
    q_enq(pool->idx_queue, idx);
}

void qpool_free(query_pool_t *pool) {
    q_free(pool->idx_queue);
    free(pool->pool);
    free(pool);
}
