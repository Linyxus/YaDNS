//
// Created by Yichen Xu on 2020/8/23.
//

#include <server/dns_server.h>
#include <utils/logging.h>

void dns_server_init(ap_t *args) {
    loop_init();
    logi("libuv event loop initialized");
    pools_init(args);
    logi("pools initialized");
    db_cache_init(args);
    logi("hosts db and cache initialized");
    sock_init(args);
    logi("udp socket initialized");
    curl_init(args);
    logi("doh conn initialized");
}

int dns_server_run() {
    return uv_run(loop, UV_RUN_DEFAULT);
}

void dns_server_deinit() {
    db_cache_deinit();
    pools_deinit();
    curl_deinit();
}

