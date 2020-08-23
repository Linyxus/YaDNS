//
// Created by Yichen Xu on 2020/6/25.
//

#ifndef DNS_RELAY_DB_TYPES_H
#define DNS_RELAY_DB_TYPES_H

#include <stdint.h>

#define DN_LABEL_MAX_LEN 64
#define DN_NAME_MAX_LEN 256

typedef uint16_t db_size_t;
typedef uint32_t db_ip_t;

struct dn_db_label {
    db_size_t len;
    char *label;
};
typedef struct dn_db_label dn_db_label_t;

struct dn_db_name {
    dn_db_label_t label;
    struct dn_db_name *next;
};
typedef struct dn_db_name dn_db_name_t;

struct dn_db_record {
    dn_db_name_t *name;
    db_ip_t ip;
};
typedef struct dn_db_record dn_db_record_t;

void destroy_name(dn_db_name_t *p);

#endif //DNS_RELAY_DB_TYPES_H
