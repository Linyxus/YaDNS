//
// Created by Yichen Xu on 2020/8/23.
//

#ifndef DNS_RELAY_CACHE_H
#define DNS_RELAY_CACHE_H

#include <db/types.h>

#define DB_CACHE_INDEX_WIDTH 0x08
#define DB_CACHE_ENTRY_NUM (0x1 << DB_CACHE_INDEX_WIDTH)
#define DB_CACHE_WAY_NUM 0x2

typedef struct {
    dn_db_name_t *key;
    dn_db_record_t *rec;
} cache_entry_t;

typedef struct {
    char lru[DB_CACHE_ENTRY_NUM];
    cache_entry_t entries[DB_CACHE_WAY_NUM][DB_CACHE_ENTRY_NUM];
} cache_t;

cache_t *cache_init();

void cache_insert(cache_t *c, dn_db_name_t *key, dn_db_record_t *val);

dn_db_record_t *cache_lookup(cache_t *c, dn_db_name_t *key);

void cache_free(cache_t *c);

uint8_t hash_name(dn_db_name_t *name);

#endif //DNS_RELAY_CACHE_H
