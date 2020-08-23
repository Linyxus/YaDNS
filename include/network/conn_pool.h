//
// Created by Yichen Xu on 2020/8/19.
//

#ifndef DNS_RELAY_CONN_POOL_H
#define DNS_RELAY_CONN_POOL_H

#include "curl/curl.h"
#include "utils/queue.h"
#include "query_pool.h"

#define CONN_CONTEXT_BUF_SIZE 512

typedef struct {
    CURL *easy_handle;
    int query_id; // associated query id
    char read_buf[CONN_CONTEXT_BUF_SIZE];
    char send_buf[CONN_CONTEXT_BUF_SIZE];
    size_t send_len;
    size_t nread;
    uint16_t dns_id;
    int conn_id;
} conn_context_t;

typedef struct {
    conn_context_t **pool;
    size_t pool_size;
    queue_t *idx_queue;
    uint16_t next_dns_id;
} conn_pool_t;

conn_pool_t *cpool_init(size_t size, char *doh_server);

int cpool_full(conn_pool_t *pool);

conn_context_t *cpool_add_conn(conn_pool_t *pool, int q_id, query_t *q);

void cpool_finish(conn_pool_t *pool, int c_id);

void cpool_free(conn_pool_t *pool);

#endif //DNS_RELAY_CONN_POOL_H
