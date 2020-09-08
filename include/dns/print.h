//
// Created by Yichen Xu on 2020/9/7.
//

#ifndef YADNS_PRINT_H
#define YADNS_PRINT_H

#include <stdio.h>
#include <dns/types.h>

void print_dns_msg(dns_msg_t *msg);

void print_dns_header(dns_msg_header_t *header);

void print_dns_domain_name(dn_name_t *name, char *orig_msg);

void print_dns_question(dns_msg_q_t *q, char *orig_msg);

void print_dns_rr(dns_msg_rr_t *rr, char *orig_msg);

#endif //YADNS_PRINT_H
