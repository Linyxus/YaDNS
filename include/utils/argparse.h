//
// Created by Yichen Xu on 2020/8/15.
//

#ifndef DNS_RELAY_ARGPARSE_H
#define DNS_RELAY_ARGPARSE_H

#include "utils/total_map.h"

typedef enum { AP_INT, AP_STORE_TRUE, AP_STR, AP_DOUBLE } ap_field_type_t;

typedef struct {
    total_map_t fields;
    total_map_t results;
    total_map_t desc;
    int parsed;
} ap_t;

ap_t *ap_init();

void ap_add_argument(ap_t *ap, char *field_key, ap_field_type_t field_type, char *desc, void *field_default);

void ap_parse(ap_t *ap, int argc, const char **argv);

int ap_get_int(ap_t *ap, char *field_key);

char *ap_get_str(ap_t *ap, char *field_key);

double ap_get_double(ap_t *ap, char *field_key);

void ap_print_help(ap_t *ap);

void ap_free(ap_t *ap);

#endif //DNS_RELAY_ARGPARSE_H
