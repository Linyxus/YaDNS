//
// Created by Yichen Xu on 2020/8/23.
//

#include <network/udp_pool.h>
#include <time.h>

udp_pool_t *upool_init(size_t size) {
    udp_pool_t *pool = malloc(sizeof(udp_pool_t));
    pool->pool_size = size;
    pool->next_dns_id = time(0);
    // init pool
    pool->pool = malloc(sizeof(void *) * size);
    for (size_t i = 0; i < size; i++) {
        pool->pool[i] = malloc(sizeof(udp_context_t));
        pool->pool[i]->valid = 0;
    }
    // init idx queue
    pool->idx_queue = q_init(size);
    for (size_t i = 0; i < size; i++) {
        q_enq(pool->idx_queue, i);
    }

    return pool;
}

int upool_full(udp_pool_t *pool) {
    return q_empty(pool->idx_queue);
}

int upool_add(udp_pool_t *pool, int q_id, query_t *q) {
    assert(!upool_full(pool));
    int u_id = q_deq(pool->idx_queue);
    pool->pool[u_id]->valid = 1;
    pool->pool[u_id]->query_id = q_id;
    pool->pool[u_id]->dns_id = pool->next_dns_id++;
    pool->pool[u_id]->send_len = q->msg_len;
    memcpy(pool->pool[u_id]->send_buf, q->msg, q->msg_len);
    set_dns_id(pool->pool[u_id]->send_buf, pool->pool[u_id]->dns_id);

    return u_id;
}

void upool_finish(udp_pool_t *pool, int u_id) {
    assert(!q_in(pool->idx_queue, u_id));
    q_enq(pool->idx_queue, u_id);
    pool->pool[u_id]->valid = 0;
}

void upool_free(udp_pool_t *pool) {
    q_free(pool->idx_queue);
    for (size_t i = 0; i < pool->pool_size; i++) {
        free(pool->pool[i]);
    }
    free(pool->pool);
    free(pool);
}
