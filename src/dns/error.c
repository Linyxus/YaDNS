//
// Created by Yichen Xu on 2020/6/24.
//

#include <stdio.h>
#include "dns/utils.h"

void print_qtype(uint16_t t) {
    switch (t) {
        case DNS_QTYPE_A:
            printf("A");
            break;
        case DNS_QTYPE_NS:
            printf("NS");
            break;
        case DNS_QTYPE_MD:
            printf("MD");
            break;
        case DNS_QTYPE_MF:
            printf("MF");
            break;
        case DNS_QTYPE_CNAME:
            printf("CNAME");
            break;
        case DNS_QTYPE_SOA:
            printf("SOA");
            break;
        case DNS_QTYPE_MB:
            printf("MB");
            break;
        case DNS_QTYPE_MG:
            printf("MG");
            break;
        case DNS_QTYPE_MR:
            printf("MR");
            break;
        case DNS_QTYPE_NULL:
            printf("NULL");
            break;
        case DNS_QTYPE_WKS:
            printf("WKS");
            break;
        case DNS_QTYPE_PTR:
            printf("PTR");
            break;
        case DNS_QTYPE_HINFO:
            printf("HINFO");
            break;
        case DNS_QTYPE_MINFO:
            printf("MINFO");
            break;
        case DNS_QTYPE_MX:
            printf("MX");
            break;
        case DNS_QTYPE_TXT:
            printf("TXT");
            break;
        case DNS_QTYPE_AXFR:
            printf("AXFR");
            break;
        case DNS_QTYPE_MAILB:
            printf("MAILB");
            break;
        case DNS_QTYPE_MAILA:
            printf("MAILA");
            break;
        case DNS_QTYPE_ASTERISK:
            printf("*");
            break;
        default:
            break;
    }
}

void print_qclass(uint16_t t) {
    switch (t) {
        case DNS_QCLASS_IN:
            printf("IN");
            break;
        case DNS_QCLASS_CS:
            printf("CS");
            break;
        case DNS_QCLASS_CH:
            printf("CH");
            break;
        case DNS_QCLASS_HS:
            printf("HS");
            break;
        case DNS_QCLASS_ASTERISK:
            printf("*");
            break;
        default:
            break;
    }
}

void print_question(const dns_msg_q_t *q, const char *raw_msg) {
    printf("Question: ");
    print_qclass(q->class);
    printf(" ");
    print_qtype(q->type);
    printf(" ");

    for (int i = 0; i < q->name.len; i++) {
        for (int j = 0; j < q->name.labels[i].len; j++) {
            int k = q->name.labels[i].offset + j;
            putchar(raw_msg[k]);
        }
        putchar('.');
    }
}
