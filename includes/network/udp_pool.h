//
// Created by Yichen Xu on 2020/8/23.
//

#ifndef DNS_RELAY_UDP_POOL_H
#define DNS_RELAY_UDP_POOL_H

#define UDP_CONTEXT_BUF_SIZE 512

#include <stdlib.h>
#include <utils/queue.h>
#include <network/query_pool.h>

typedef struct {
    int query_id;
    int dns_id;
    char send_buf[UDP_CONTEXT_BUF_SIZE];
    size_t send_len;
    char valid;
} udp_context_t;

typedef struct {
    udp_context_t **pool;
    size_t pool_size;
    queue_t *idx_queue;
    uint16_t next_dns_id;
} udp_pool_t;

udp_pool_t *upool_init(size_t size);

int upool_full(udp_pool_t *pool);

int upool_add(udp_pool_t *pool, int q_id, query_t *q);

void upool_finish(udp_pool_t *pool, int u_id);

void upool_free(udp_pool_t *pool);

#endif //DNS_RELAY_UDP_POOL_H
