//
// Created by Yichen Xu on 2020/8/22.
//

#ifndef DNS_RELAY_COMPOSE_H
#define DNS_RELAY_COMPOSE_H

#include <dns/types.h>
#include <stdlib.h>

char *compose_a_rr(dn_name_t *dn_name, int32_t ip, uint32_t ttl, size_t *len);

char *compose_a_rr_ans(char *raw, size_t raw_len, char *a_rr, size_t rr_len, size_t *len);

char *compose_header(dns_msg_header_t *header, size_t *msg_len);

#endif //DNS_RELAY_COMPOSE_H
