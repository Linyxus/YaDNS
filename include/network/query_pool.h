//
// Created by Yichen Xu on 2020/8/18.
//

#ifndef DNS_RELAY_QUERY_POOL_H
#define DNS_RELAY_QUERY_POOL_H

#include <sys/socket.h>
#include "dns/parse.h"
#include "utils/queue.h"

typedef struct {
    struct sockaddr addr;
    char *msg;
    size_t msg_len;
    uint16_t dns_id;
} query_t;

typedef struct {
    query_t *pool;
    size_t pool_size;
    queue_t *idx_queue;
} query_pool_t;

query_pool_t *qpool_init(size_t pool_size);

int qpool_full(query_pool_t *pool);

int qpool_insert(query_pool_t *pool, struct sockaddr addr, char *msg, size_t msg_len);

void qpool_remove(query_pool_t *pool, size_t idx);

void qpool_free(query_pool_t *pool);

#endif //DNS_RELAY_QUERY_POOL_H
