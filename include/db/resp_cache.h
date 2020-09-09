//
// Created by Yichen Xu on 2020/9/8.
//

#ifndef YADNS_RESP_CACHE_H
#define YADNS_RESP_CACHE_H

#include <db/types.h>
#include <utils/queue.h>
#include <utils/trie.h>
#include <utils/time.h>
#include <time.h>

#define LRU_CACHE_SIZE 256

typedef struct resp_cache_node_s {
    dn_db_record_t *record;
    time_t expire_at;
} resp_cache_node_t;

char *compose_trie_key(dn_db_name_t *name);

trie trie_insert_db_dn(trie t, dn_db_name_t *name, void *val);

trie rc_init();

void rc_insert(trie t, dn_db_name_t *name, db_ip_t ip, uint32_t ttl);

resp_cache_node_t * rc_lookup(trie t, dn_db_name_t *name);

resp_cache_node_t *rc_node_init(dn_db_name_t *name, db_ip_t ip, uint32_t ttl);

void rc_node_deinit(resp_cache_node_t *node);

#endif //YADNS_RESP_CACHE_H
