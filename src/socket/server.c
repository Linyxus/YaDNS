//
// Created by Yichen Xu on 2020/5/28.
//

#include <socket/server.h>

udp_create_t udp_create(unsigned short port, int *sock_fd) {
    struct sockaddr_in addr;
    int fd;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return UDP_CREATE_SOCKET_ERROR;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(fd, (const struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return UDP_CREATE_BIND_ERROR;
    }

    *sock_fd = fd;

    return UDP_CREATE_OK;
}

void udp_close(int sock_fd) {
    close(sock_fd);
}

void udp_recv(int sock_fd, struct udp_cli_msg *msg) {
    char *buffer = msg->data;
    unsigned int max_len = MAX_MSG_LEN;
    struct sockaddr *cli_addr = (struct sockaddr *)&(msg->addr.addr);
    socklen_t addr_len;

    ssize_t len = recvfrom(sock_fd, buffer, max_len, 0, cli_addr, &addr_len);
    buffer[len] = '\0';

    inet_ntop(AF_INET, &(msg->addr.addr.sin_addr), msg->addr.ip, INET_ADDRSTRLEN);
    msg->addr.port = ntohs(msg->addr.addr.sin_port);
    msg->addr.addr_len = addr_len;
    msg->msg_len = len;
}

void udp_send(int sock_fd, const udp_addr_t *addr, char *msg, unsigned int msg_len) {
    sendto(sock_fd, msg, msg_len, 0, (struct sockaddr *)&addr->addr, addr->addr_len);
}

void make_addr(const char *ip, uint16_t port, udp_addr_t *addr) {
    strcpy(addr->ip, ip);
    addr->port = port;
    resolve_addr(addr);
}

void resolve_addr(udp_addr_t *addr) {
    addr->addr.sin_family = AF_INET;
    addr->addr.sin_port = htons(addr->port);
    inet_aton(addr->ip, &addr->addr.sin_addr);
    addr->addr_len = sizeof(addr->addr);
    memset(addr->addr.sin_zero, 0, sizeof(addr->addr.sin_zero));
}

void print_addr(const udp_addr_t *addr) {
    printf("%s::%d", addr->ip, addr->port);
}

