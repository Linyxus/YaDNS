//
// Created by Yichen Xu on 2020/9/7.
//

#ifndef DNS_RELAY_TRIE_H
#define DNS_RELAY_TRIE_H

#define TRIE_TREE_MAX_LEAF 32

typedef struct trie_node_s {
    char key;
    void *val;
    struct trie_node_s *children[TRIE_TREE_MAX_LEAF];
    int num_child;
} trie_node_t;

typedef trie_node_t *trie;

trie trie_init();

void *trie_lookup(trie t, char *key);

trie trie_insert(trie t, char *key, void *val);

void trie_deinit(trie t);

char trie_collect_garbage(trie t);

char trie_empty_tree(trie t);


#endif //DNS_RELAY_TRIE_H
