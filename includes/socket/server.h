//
// Created by Yichen Xu on 2020/5/28.
//

#ifndef DNS_RELAY_SERVER_H
#define DNS_RELAY_SERVER_H

#include "types.h"

#define UDP_CREATE_OK 0
#define UDP_CREATE_BIND_ERROR 1
#define UDP_CREATE_SOCKET_ERROR 2

typedef int udp_create_t;

udp_create_t udp_create(unsigned short port, int *sock_fd);
void udp_close(int sock_fd);
void udp_recv(int sock_fd, struct udp_cli_msg *msg);
void udp_send(int sock_fd, const udp_addr_t *addr, char *msg, unsigned int msg_len);

void make_addr(const char *ip, uint16_t port, udp_addr_t *addr);
void resolve_addr(udp_addr_t *addr);
void print_addr(const udp_addr_t *addr);

#endif //DNS_RELAY_SERVER_H
