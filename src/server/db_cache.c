//
// Created by Yichen Xu on 2020/8/23.
//

#include <server/db_cache.h>
#include <db/io.h>
#include <utils/logging.h>
#include <utils/error.h>

tree_t *db_tree = 0;
cache_t *db_cache = 0;
static dn_db_record_t *db_rec = 0;

void db_cache_init(ap_t *ap) {
    int count = 0;
    int db_ret_code = 0;
    db_rec = db_read_all_records("/Users/linyxus/dev/dns-relay/hosts.txt", &count, &db_ret_code);
    if (db_ret_code != DB_PARSE_RECORD_EOF) {
        loge("Error parsing hosts file.");
        error("Fatal error when starting server.");
    }
    db_tree = tree_build_from_rec(db_rec, count);
    db_cache = cache_init();
}

void db_cache_deinit() {
    free(db_rec);
}
