//
// Created by Yichen Xu on 2020/8/23.
//

#ifndef DNS_RELAY_DB_CACHE_H
#define DNS_RELAY_DB_CACHE_H

#include <db/tree.h>
#include <db/cache.h>
#include <db/lru_cache.h>
#include <utils/argparse.h>

extern tree_t *db_tree;
extern cache_t *db_cache;
extern trie db_lru_cache;

void db_cache_init(ap_t *ap);

void db_cache_deinit();

#endif //DNS_RELAY_DB_CACHE_H
