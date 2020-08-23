//
// Created by Yichen Xu on 2020/6/25.
//

#ifndef DNS_RELAY_DB_IO_H
#define DNS_RELAY_DB_IO_H

#define DB_PARSE_MAX_RECORDS 2048

#include <db/types.h>

dn_db_record_t *db_read_all_records(char *path, int *count, int *ret_code);

#endif //DNS_RELAY_DB_IO_H
