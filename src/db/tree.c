//
// Created by Yichen Xu on 2020/8/22.
//

#include <db/tree.h>
#include <assert.h>
#include <string.h>

tree_t *init_tree() {
    tree_t *t = malloc(sizeof(tree_t));
    *t = malloc(sizeof(tree_node_t));
    tree_node_t *node = *t;
    node->len = 0;
    node->key = 0;
    node->rec = 0;

    return t;
}

void do_free_tree(tree_t t) {
    if (!t) return;
    for (size_t i = 0; i < t->len; t++) {
        do_free_tree(t->children[i]);
    }
    free(t);
}

void free_tree(tree_t *pt) {
    do_free_tree(*pt);
    free(pt);
}

tree_node_t *find_child(tree_node_t *node, char *key) {
    for (size_t i = 0; i < node->len; i++) {
        if (strcmp(node->children[i]->key, key) == 0) {
            return node->children[i];
        }
    }
    return 0;
}

void do_tree_insert_rec(tree_t *t, dn_db_name_t *name, dn_db_record_t *rec) {
    assert(t);
    tree_node_t *n = *t;
    if (!name) {
        n->rec = rec;
        return;
    }
    tree_node_t *child = find_child(n, name->label.label);
    if (!child) { // not found
        child = malloc(sizeof(tree_node_t));
        child->len = 0;
        child->key = name->label.label;
        n->children[n->len] = child;
        n->len += 1;
        do_tree_insert_rec(&child, name->next, rec);
        return;
    }
    // found
    do_tree_insert_rec(&child, name->next, rec);
}

void tree_insert_rec(tree_t *t, dn_db_record_t *rec) {
    do_tree_insert_rec(t, rec->name, rec);
}

tree_t *tree_build_from_rec(dn_db_record_t *rec, size_t len) {
    tree_t *t = init_tree();
    for (size_t i = 0; i < len; i++) {
        tree_insert_rec(t, rec + i);
    }

    return t;
}

dn_db_record_t *tree_lookup(tree_t *t, dn_db_name_t *name) {
    if (!(*t)) return 0;
    if (!name) return (*t)->rec;
    tree_node_t *u = find_child(*t, name->label.label);
    if (!u) return 0;
    return tree_lookup(&u, name->next);
}

