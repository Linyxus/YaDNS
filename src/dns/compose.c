//
// Created by Yichen Xu on 2020/8/22.
//

#include <dns/compose.h>
#include <assert.h>
#include "string.h"

char *compose_a_rr(dn_name_t *dn_name, int32_t ip, uint32_t ttl, size_t *len) {
    char *rr = malloc(sizeof(char) * 16);
    assert(dn_name->len);
    *len = 16;
    // set domain name
    *(uint16_t *) rr = htons((dn_name->labels[0].offset - 1) | 0xc000);
    char *p = rr;
    // set type
    p += 2;
    *(uint16_t *) p = htons(DNS_QTYPE_A);
    // set class
    p += 2;
    *(uint16_t *) p = htons(DNS_QCLASS_IN);
    // set ttl
    p += 2;
    *(uint32_t *) p = htonl(ttl);
    // set rd length
    p += 4;
    *(uint16_t *) p = htons(4);
    // set ip
    p += 2;
    *(uint32_t *) p = htonl(ip);

    return rr;
}

char *compose_header(dns_msg_header_t *header, size_t *msg_len) {
    char *msg = malloc(12 * sizeof(char));
    *msg_len = 12;
    char *p = msg;

    *(uint16_t *) p = htons(header->id);
    p += 2;
    *(uint8_t *) p = 0;
    *(uint8_t *) p = (header->qr << 7) | (header->opcode << 3) | (header->aa << 2) | header->tc
                     | header->rd;
    p += 1;

    *(uint8_t *) p = 0;
    *(uint8_t *) p = (header->ra << 7) | header->rcode;
    p += 1;

    *(uint16_t *) p = htons(header->qd_cnt);
    p += 2;

    *(uint16_t *) p = htons(header->an_cnt);
    p += 2;

    *(uint16_t *) p = htons(header->ns_cnt);
    p += 2;

    *(uint16_t *) p = htons(header->ar_cnt);

    return msg;
}

char *compose_a_rr_ans(char *raw, size_t raw_len, char *a_rr, size_t rr_len, size_t *len) {
    *len = raw_len + rr_len;
    char *msg = malloc(sizeof(char) * *len);
    memcpy(msg, raw, raw_len);
    memcpy(msg + raw_len, a_rr, rr_len);

    // set qr
    *(uint8_t *) (msg + 2) |= 0x80;
    // set rcode
    *(uint8_t *) (msg + 3) &= 0xf0;
    // set ans count
    *(uint16_t *) (msg + 6) = htons(1);
    // clear additional count
    *(uint16_t *) (msg + 10) = htons(0);

    return msg;
}

