//
// Created by Yichen Xu on 2020/8/23.
//

#ifndef DNS_RELAY_CURL_H
#define DNS_RELAY_CURL_H

#include <curl/curl.h>
#include <uv.h>
#include <utils/argparse.h>

void add_doh_connection(struct sockaddr addr, char *req_data, size_t req_len);

void curl_init(ap_t *args);

void curl_deinit();

#endif //DNS_RELAY_CURL_H
