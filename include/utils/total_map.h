//
// Created by Yichen Xu on 2020/8/15.
//

#ifndef DNS_RELAY_TOTAL_MAP_H
#define DNS_RELAY_TOTAL_MAP_H

typedef enum { INT_TYPE, STR_TYPE, DOUBLE_TYPE } node_type_t;

struct total_map_node {
    struct total_map_node *next;
    char *key;
    void *value;
    node_type_t type;
};

typedef struct total_map_node total_map_node_t;

typedef total_map_node_t *total_map_t;

total_map_t tm_insert_int(total_map_t m, char *key, int val);
total_map_t tm_insert_str(total_map_t m, char *key, char *val);
total_map_t tm_insert_double(total_map_t m, char *key, double val);

int *tm_get_int(total_map_t m, char *key);
char *tm_get_str(total_map_t m, char *key);
double *tm_get_double(total_map_t m, char *key);
void *tm_get_any(total_map_t m, char *key);

void tm_free(total_map_t m);

#endif //DNS_RELAY_TOTAL_MAP_H
