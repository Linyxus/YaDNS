//
// Created by Yichen Xu on 2020/8/23.
//

#ifndef DNS_RELAY_DNS_SERVER_H
#define DNS_RELAY_DNS_SERVER_H

#include <server/pools.h>
#include <server/loop.h>
#include <server/db_cache.h>
#include <server/socket.h>
#include <server/curl.h>

void dns_server_init(ap_t *args);

int dns_server_run();

void dns_server_deinit();

#endif //DNS_RELAY_DNS_SERVER_H
