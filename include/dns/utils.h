//
// Created by Yichen Xu on 2020/6/24.
//

#ifndef DNS_RELAY_UTILS_H
#define DNS_RELAY_UTILS_H

#include <dns/types.h>

void print_qtype(uint16_t t);
void print_qclass(uint16_t t);

void print_question(const dns_msg_q_t *q, const char *raw_msg);

#endif //DNS_RELAY_UTILS_H
