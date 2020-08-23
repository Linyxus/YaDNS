//
// Created by Yichen Xu on 2020/5/29.
//

#ifndef DNS_RELAY_SOCKET_TYPES_H
#define DNS_RELAY_SOCKET_TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_MSG_LEN 1024

struct udp_addr {
    struct sockaddr_in addr;
    socklen_t addr_len;
    char ip[INET_ADDRSTRLEN];
    unsigned short port;
};

struct udp_cli_msg {
    struct udp_addr addr;
    char data[MAX_MSG_LEN];
    unsigned int msg_len;
};

typedef struct udp_addr udp_addr_t;
typedef struct udp_cli_msg udp_cli_msg_t;

#endif //DNS_RELAY_SOCKET_TYPES_H
