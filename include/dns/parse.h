//
// Created by Yichen Xu on 2020/5/29.
//

#ifndef DNS_RELAY_PARSE_H
#define DNS_RELAY_PARSE_H

#include "types.h"
#include <arpa/inet.h>

#define DN_PARSE_LABEL_OKAY 0
#define DN_PARSE_LABEL_INVALID_LEN 1
#define DN_PARSE_LABEL_EON 2

#define DN_PARSE_NAME_OKAY 0
#define DN_PARSE_NAME_INVALID 1

#define DNS_MSG_PARSE_Q_OKAY 0
#define DNS_MSG_PARSE_Q_INVALID 1

#define DNS_MSG_PARSE_OKAY 0
#define DNS_MSG_PARSE_INVALID 1

char *parse_header(char *msg, dns_msg_header_t *header);
char *parse_name(char *current, char *orig_msg, dn_name_t *name, int *ret_code);
char *parse_question(char *current, char *orig_msg, dns_msg_q_t *q, int *ret_code);
int parse_dns_msg(char *msg, dns_msg_t *res);

uint16_t get_dns_id(char *msg);
void set_dns_id(char *msg, uint16_t dns_id);

#endif //DNS_RELAY_PARSE_H
