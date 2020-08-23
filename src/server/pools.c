//
// Created by Yichen Xu on 2020/8/23.
//

#include <server/pools.h>

query_pool_t *qpool = 0;
conn_pool_t *cpool = 0;
udp_pool_t *upool = 0;

void pools_init(ap_t *args) {
    qpool = qpool_init(ap_get_int(args, "max_query"));
    cpool = cpool_init(ap_get_int(args, "max_doh_conn"), ap_get_str(args, "doh_server"));
    upool = upool_init(ap_get_int(args, "max_udp_req"));
}

void pools_deinit() {
    qpool_free(qpool);
    cpool_free(cpool);
    upool_free(upool);
}
