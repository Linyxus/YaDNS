//
// Created by Yichen Xu on 2020/8/23.
//

#ifndef DNS_RELAY_SOCKET_H
#define DNS_RELAY_SOCKET_H

#include <uv.h>
#include <utils/argparse.h>

#define UDP_TIMEOUT 1000

extern uv_udp_t *srv_sock, *cli_sock;

void sock_init(ap_t *ap);

void on_send(uv_udp_send_t* handle, int status);

#endif //DNS_RELAY_SOCKET_H
