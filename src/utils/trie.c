//
// Created by Yichen Xu on 2020/9/7.
//

#include <utils/trie.h>
#include <stdlib.h>

trie trie_init() {
    trie t = malloc(sizeof(trie_node_t));
    t->val = 0;
    t->key = '\0';
    t->num_child = 0;
    return t;
}

void trie_deinit(trie t) {
    if (!t) return;
    for (int i = 0; i < t->num_child; i++) {
        trie_deinit(t->children[i]);
    }
    free(t);
}

void *trie_lookup(trie t, char *key) {
    if (!t) return 0;
    if (*key == '\0') return t->val;
    for (int i = 0; i < t->num_child; i++) {
        if (*key == t->children[i]->key)
            return trie_lookup(t->children[i], key + 1);
    }
    return 0;
}

trie trie_insert(trie t, char *key, void *val) {
    if (*key == '\0') {
        t->val = val;
        return t;
    }
    if (!t) {
        return 0;
    }
    for (int i = 0; i < t->num_child; i++) {
        if (*key == t->children[i]->key) {
            return trie_insert(t->children[i], key + 1, val);
        }
    }
    trie u = malloc(sizeof(struct trie_node_s));
    u->num_child = 0;
    u->key = *key;
    t->children[t->num_child++] = u;
    return trie_insert(u, key + 1, val);
}
