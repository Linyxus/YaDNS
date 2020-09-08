//
// Created by Yichen Xu on 2020/9/8.
//

#include <db/lru_cache.h>
#include <string.h>
#include <stdlib.h>

char *compose_trie_key(dn_db_name_t *name) {
    int len = 0;
    dn_db_name_t *u = name;
    while (u) {
        len += u->label.len;
        len += 1;
        u = u->next;
    }
    len -= 1;
    char *key = malloc(len * sizeof(char));
    u = name;
    int x = 0;
    while (u) {
        memcpy(key + x, u->label.label, u->label.len);
        key[x + u->label.len] = '.';
        x += u->label.len + 1;
        u = u->next;
    }
    key[len] = '\0';

    return key;
}

trie trie_insert_db_dn(trie t, dn_db_name_t *name, void *val) {
    char *key = compose_trie_key(name);
    time(0);

    return trie_insert(t, key, val);
}

trie lc_init() {
    return trie_init();
}

void lc_insert(trie t, dn_db_name_t *name, db_ip_t ip, uint32_t ttl) {
    void *u = lc_node_init(name, ip, ttl);
    char *k = compose_trie_key(name);
    void *v = trie_lookup(t, k);
    if (v) {
        lc_node_deinit(v);
    }
    trie_insert_db_dn(t, name, u);

    free(k);
}

db_ip_t lc_lookup(trie t, dn_db_name_t *name) {
    char *key = compose_trie_key(name);
    lru_cache_node_t *node = trie_lookup(t, key);
    if (!node) return 0;
    if (node->expire_at <= now()) {
        lc_node_deinit(node);
        trie_insert(t, key, 0);
        trie_collect_garbage(t);
        return 0;
    }
    free(key);
    return node->record->ip;
}

lru_cache_node_t *lc_node_init(dn_db_name_t *name, db_ip_t ip, uint32_t ttl) {
    lru_cache_node_t *node = malloc(sizeof(lru_cache_node_t));
    node->expire_at = now() + ttl;
    node->record = malloc(sizeof(dn_db_record_t));
    node->record->name = name;
    node->record->ip = ip;

    return node;
}

void lc_node_deinit(lru_cache_node_t *node) {
    destroy_name(node->record->name);
    free(node->record);
    free(node);
}
