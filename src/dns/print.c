//
// Created by Yichen Xu on 2020/9/7.
//

#include <dns/print.h>
#include <utils/logging.h>

void print_dns_domain_name(dn_name_t *name, char *orig_msg) {
    for (int i = 0; i < name->len; i++) {
        for (int j = 0; j < name->labels[i].len; j++) {
            putchar(orig_msg[name->labels[i].offset + j]);
        }
        putchar('.');
    }
}

void print_dns_header(dns_msg_header_t *header) {
    printf("id %d, qr %d, opcode %d, aa %d, tc %d, rd %d, ra %d,"
           " rcode %d, qdcount %d, ancount %d, nscount %d, arcount %d",
           header->id, header->qr, header->opcode, header->aa, header->tc,
           header->rd, header->ra, header->rcode, header->qd_cnt,
           header->an_cnt, header->ns_cnt, header->ar_cnt
    );
}

void print_dns_question(dns_msg_q_t *q, char *orig_msg) {
    printf("question ");
    print_dns_domain_name(&q->name, orig_msg);
    printf(" type %d, class %d", q->type, q->class);
}

void print_dns_rr(dns_msg_rr_t *rr, char *orig_msg) {
    printf("rr ");
    print_dns_domain_name(&rr->name, orig_msg);
    printf(" type %d, ttl %d, rdlength %d", rr->type, rr->ttl, rr->rdlength);
}

void print_dns_msg(dns_msg_t *msg) {
    puts("=== Header ===");
    print_dns_header(&msg->header);
    putchar('\n');
    puts("=== Questions ===");
    for (int i = 0; i < msg->header.qd_cnt; i++) {
        print_dns_question(msg->question + i, msg->raw);
        putchar('\n');
    }
    puts("=== Answer ===");
    for (int i = 0; i < msg->header.an_cnt; i++) {
        print_dns_rr(msg->answer + i, msg->raw);
        putchar('\n');
    }
    puts("=== Authority ===");
    for (int i = 0; i < msg->header.ns_cnt; i++) {
        print_dns_rr(msg->authority + i, msg->raw);
        putchar('\n');
    }
    puts("=== Additional ===");
    for (int i = 0; i < msg->header.ar_cnt; i++) {
        print_dns_rr(msg->additional + i, msg->raw);
        putchar('\n');
    }
}
