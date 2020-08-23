//
// Created by Yichen Xu on 2020/6/25.
//

#ifndef DNS_RELAY_DB_PARSE_H
#define DNS_RELAY_DB_PARSE_H

#include <db/types.h>

#define DB_PARSE_NAME_OKAY 0
#define DB_PARSE_NAME_INVALID 1

#define DB_PARSE_IP_OKAY 0
#define DB_PARSE_IP_INVALID 1

#define DB_PARSE_RECORD_OKAY 0
#define DB_PARSE_RECORD_INVALID 1
#define DB_PARSE_RECORD_EOF 2

dn_db_name_t *db_parse_name(const char *str, uint16_t len, int *ret_code);
db_ip_t db_parse_ip(const char *str, uint16_t len, int *ret_code);
int db_parse_next_record(char **p, char *end, dn_db_record_t *res);

#endif //DNS_RELAY_DB_PARSE_H
