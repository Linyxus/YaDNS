//
// Created by Yichen Xu on 2020/8/23.
//

#ifndef DNS_RELAY_POOLS_H
#define DNS_RELAY_POOLS_H

#include <network/conn_pool.h>
#include <network/query_pool.h>
#include <network/udp_pool.h>
#include <utils/argparse.h>

extern query_pool_t *qpool;
extern conn_pool_t *cpool;
extern udp_pool_t *upool;

void pools_init(ap_t *args);

void pools_deinit();

#endif //DNS_RELAY_POOLS_H
