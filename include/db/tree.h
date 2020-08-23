//
// Created by Yichen Xu on 2020/8/22.
//

#ifndef DNS_RELAY_TREE_H
#define DNS_RELAY_TREE_H

#include <db/parse.h>
#include <stdlib.h>

struct tree_node_s {
    char *key;
    dn_db_record_t *rec;
    struct tree_node_s *children[256];
    size_t len;
};
typedef struct tree_node_s tree_node_t;

typedef tree_node_t *tree_t;

tree_t *init_tree();

void tree_insert_rec(tree_t *t, dn_db_record_t *rec);

dn_db_record_t *tree_lookup(tree_t *t, dn_db_name_t *name);

tree_t *tree_build_from_rec(dn_db_record_t *rec, size_t len);

void free_tree(tree_t *t);

#endif //DNS_RELAY_TREE_H
