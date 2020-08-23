//
// Created by Yichen Xu on 2020/8/15.
//

#include "utils/total_map.h"
#include <stdlib.h>
#include <string.h>

total_map_t tm_insert_int(total_map_t m, char *key, int val) {
    total_map_t ret = (total_map_t) malloc(sizeof(total_map_node_t));
    ret->type = INT_TYPE;
    ret->key = malloc(strlen(key));
    strcpy(ret->key, key);
    ret->value = malloc(sizeof(int));
    *((int *) ret->value) = val;
    ret->next = m;

    return ret;
}

total_map_t tm_insert_str(total_map_t m, char *key, char *val) {
    total_map_t ret = (total_map_t) malloc(sizeof(total_map_node_t));
    ret->type = STR_TYPE;
    ret->key = malloc(strlen(key) + 1);
    strcpy(ret->key, key);
    ret->key[strlen(key)] = 0;
    ret->value = malloc(strlen(val) + 1);
    strcpy(ret->value, val);
    ((char *) ret->value)[strlen(val)] = 0;
    ret->next = m;

    return ret;
}

total_map_t tm_insert_double(total_map_t m, char *key, double val) {
    total_map_t ret = (total_map_t) malloc(sizeof(total_map_node_t));
    ret->type = DOUBLE_TYPE;
    ret->key = malloc(strlen(key));
    strcpy(ret->key, key);
    ret->value = malloc(sizeof(double));
    *((double *) ret->value) = val;
    ret->next = m;

    return ret;
}

int *tm_get_int(total_map_t m, char *key) {
    return (int *) tm_get_any(m, key);
}

char *tm_get_str(total_map_t m, char *key) {
    return (char *) tm_get_any(m, key);
}

double *tm_get_double(total_map_t m, char *key) {
    return (double *) tm_get_any(m, key);
}

void *tm_get_any(total_map_t m, char *key) {
    if (!m) return NULL;
    if (strcmp(key, m->key) == 0) {
        return m->value;
    } else {
        return tm_get_any(m->next, key);
    }
}

void tm_free(total_map_t m) {
    if (!m) return;
    switch (m->type) {
        case INT_TYPE:
            free((int *) m->value);
            break;
        case STR_TYPE:
            free((char *) m->value);
            break;
        case DOUBLE_TYPE:
            free((double *) m->value);
            break;
    }
    free(m->key);
    tm_free(m->next);
    free(m);
}

