//
// Created by Yichen Xu on 2020/8/15.
//

#include "utils/argparse.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils/error.h"

ap_t *ap_init() {
    ap_t *ret = (ap_t *) malloc(sizeof(ap_t));
    ret->parsed = 0;
    ret->fields = 0;
    ret->results = 0;
    ret->desc = 0;

    return ret;
}

void ap_free(ap_t *ap) {
    tm_free(ap->fields);
    tm_free(ap->results);
    free(ap);
}

void ap_add_argument(ap_t *ap, char *field_key, ap_field_type_t field_type, char *desc, void *field_default) {
    if (ap->parsed) {
        error("Can not add argument to a parsed ap handle; please create a new one.\n");
    }
    ap->fields = tm_insert_int(ap->fields, field_key, field_type);
    if (field_default) {
        switch (field_type) {
            case AP_INT:
            case AP_STORE_TRUE:
                ap->results = tm_insert_int(ap->results, field_key, *(int *) field_default);
                break;
            case AP_STR:
                ap->results = tm_insert_str(ap->results, field_key, (char *) field_default);
                break;
            case AP_DOUBLE:
                ap->results = tm_insert_double(ap->results, field_key, *(double *) field_default);
                break;
            default:
                break;
        }
    } else {
        if (field_type == AP_STORE_TRUE) {
            ap->results = tm_insert_int(ap->results, field_key, 0);
        }
    }
    if (desc) {
        ap->desc = tm_insert_str(ap->desc, field_key, desc);
    }
}

void ap_parse(ap_t *ap, int argc, const char **argv) {
    int p = 0;
    char *name = 0;
    argc = argc - 1;
    argv = argv + 1;
    if (argc == 1 && strcmp(*argv, "--help") == 0) {
        ap_print_help(ap);
        exit(0);
    }
    while (p < argc) {
        name = argv[p];
        if (name[0] != '-' || name[1] != '-') {
            fprintf(stderr, "Invalid flag %s: flag should start with `--`.\n", name);
            error("Parse failed.");
        }
        name = name + 2;
        int *pfield_type = tm_get_int(ap->fields, name);
        if (!pfield_type) {
            fprintf(stderr, "Invalid flag %s: unknown flag.\n", name);
            error("Parse failed.");
        }
        ap_field_type_t field_type = *pfield_type;
        int d;
        double f;
        switch (field_type) {
            case AP_INT:
                p += 1;
                if (p == argc) {
                    fprintf(stderr, "Not enough argument: expect an argument for %s.\n", name);
                    error("Parse failed.");
                }
                if (sscanf(argv[p], "%d", &d) != 1) {
                    fprintf(stderr, "Invalid argument %s for %s: expect a int here.\n", name, argv[p]);
                    error("Parse failed.");
                }
                ap->results = tm_insert_int(ap->results, name, d);
                break;
            case AP_STORE_TRUE:
                ap->results = tm_insert_int(ap->results, name, 1);
                break;
            case AP_DOUBLE:
                p += 1;
                if (p == argc) {
                    fprintf(stderr, "Not enough argument: expect an argument for %s.\n", name);
                    error("Parse failed.");
                }
                if (sscanf(argv[p], "%lf", &f) != 1) {
                    fprintf(stderr, "Invalid argument %s for %s: expect a double here.\n", name, argv[p]);
                    error("Parse failed.");
                }
                ap->results = tm_insert_double(ap->results, name, f);
                break;
            case AP_STR:
                p += 1;
                if (p == argc) {
                    fprintf(stderr, "Not enough argument: expect an argument for %s.\n", name);
                    error("Parse failed.");
                }
                ap->results = tm_insert_str(ap->results, name, argv[p]);
        }
        p += 1;
    }

    int missing = 0;
    total_map_t m = ap->fields;
    while (m) {
        void *x = tm_get_any(ap->results, m->key);
        if (!x) {
            fprintf(stderr, "No default is provided for field %s and no value has been supplied.\n", m->key);
            missing = 1;
        }
        m = m->next;
    }
    if (missing) {
        error("Parse failed.");
    }
}

int ap_get_int(ap_t *ap, char *field_key) {
    if (!tm_get_any(ap->fields, field_key)) {
        fprintf(stderr, "Field %s is invalid.\n", field_key);
        error("Get argument failed.");
    }
    return *tm_get_int(ap->results, field_key);
}

char *ap_get_str(ap_t *ap, char *field_key) {
    if (!tm_get_any(ap->fields, field_key)) {
        fprintf(stderr, "Field %s is invalid.\n", field_key);
        error("Get argument failed.");
    }
    return tm_get_str(ap->results, field_key);
}

double ap_get_double(ap_t *ap, char *field_key) {
    if (!tm_get_any(ap->fields, field_key)) {
        fprintf(stderr, "Field %s is invalid.\n", field_key);
        error("Get argument failed.");
    }
    return *tm_get_double(ap->results, field_key);
}

void print_field_help(char *field_name, int field_type, char *field_desc) {
    printf("  --%s", field_name);
    putchar(' ');
    switch (field_type) {
        case AP_STR:
            printf("STR"); break;
        case AP_INT:
            printf("INT"); break;
        case AP_DOUBLE:
            printf("FLOAT"); break;
        default: break;
    }
    puts("\n");
    if (field_desc) {
        printf("    %s\n\n", field_desc);
    }
}

void ap_do_print_help(ap_t *ap, total_map_t field_list) {
    if (!field_list) return;
    ap_do_print_help(ap, field_list->next);
    print_field_help(field_list->key, *(int *) field_list->value, tm_get_str(ap->desc, field_list->key));
}

void ap_print_help(ap_t *ap) {
    printf("dns-relay -- Simple dns proxy, with DoH\n\n");
    print_field_help("help", AP_STORE_TRUE, "Display this helper text.");
    ap_do_print_help(ap, ap->fields);
}

