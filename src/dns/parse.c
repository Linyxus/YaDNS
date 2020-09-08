//
// Created by Yichen Xu on 2020/5/29.
//

#include <dns/parse.h>

uint16_t get_dns_id(char *msg) {
    return ntohs(*(unsigned short *) msg);
}

void set_dns_id(char *msg, uint16_t dns_id) {
    *(uint16_t *) msg = htons(dns_id);
}

char *parse_header(char *msg, dns_msg_header_t *header) {
    // parse id
    header->id = ntohs(*(unsigned short *)msg);
    msg += 2;

    // parse control bits
    uint8_t b = *(uint8_t *)msg;
    header->qr = (b >> 7) & 0x1;
    header->opcode = (b >> 3) & 0xf;
    header->aa = (b >> 2) & 0x1;
    header->tc = (b >> 1) & 0x1;
    header->rd = b & 0x1;
    msg += 1;

    b = *(uint8_t *)msg;
    header->ra = (b >> 7) & 0x1;
    header->rcode = b & 0xf;
    msg += 1;

    header->qd_cnt = ntohs(*(unsigned short *)msg);
    msg += 2;

    header->an_cnt = ntohs(*(unsigned short *)msg);
    msg += 2;

    header->ns_cnt = ntohs(*(unsigned short *)msg);
    msg += 2;

    header->ar_cnt = ntohs(*(unsigned short *)msg);
    msg += 2;

    return msg;
}

char *parse_label(char *current, char *orig_msg, dn_label_t *label, int *ret_code) {
    uint16_t len = ntohs(*(uint16_t *)current);
    char *p = 0;
    char *ret = 0;
    if (((len >> 14) & 0x3) == 0x3) { // label pointer
        uint16_t offset = len & 0x3fff;
        ret = parse_label(orig_msg + offset, orig_msg, label, ret_code);
        if (*ret_code == DN_PARSE_LABEL_OKAY) {
            *ret_code = DN_PARSE_LABEL_OKAY_PTR;
        }
    } else if (((len >> 14) & 0x3) == 0x0) {
        p = current + 1;
        len = (*(uint8_t *)current);
        if (len == 0) {
            *ret_code = DN_PARSE_LABEL_EON;
            return p;
        }
        len &= 0x3f; // ignore higher 2 bits
        label->offset = p - orig_msg;
        label->len = len;
        *ret_code = DN_PARSE_LABEL_OKAY;
        ret = p + len;
    } else {
        *ret_code = DN_PARSE_LABEL_INVALID_LEN;
        return 0;
    }
    return ret;
}

char *parse_name(char *current, char *orig_msg, dn_label_t *label, dns_size_t *name_len, int *ret_code) {
    char *p = current;
    char *q = 0;
    int pl_ret_code = DN_PARSE_LABEL_OKAY;
    int len = 0;

    while (pl_ret_code == DN_PARSE_LABEL_OKAY) {
        char *new_p;
        new_p = parse_label(p, orig_msg, label + len, &pl_ret_code);
        len = len + 1;
        if (pl_ret_code != DN_PARSE_LABEL_OKAY_PTR) {
            p = new_p;
        } else {
            p = p + 2;
            q = new_p;
        }
    }

    if (pl_ret_code == DN_PARSE_LABEL_OKAY_PTR) {
        int pn_ret_code = DN_PARSE_NAME_OKAY;
        dns_size_t ptr_len = 0;
        parse_name(q, orig_msg, label + len, &ptr_len, &pn_ret_code);
        if (pn_ret_code == DN_PARSE_NAME_INVALID) {
            *ret_code = pn_ret_code;
            return 0;
        }
        len += ptr_len;
        len += 1;
    }

    if (pl_ret_code == DN_PARSE_LABEL_INVALID_LEN) {
        *ret_code = DN_PARSE_NAME_INVALID;
        return 0;
    }
    len = len - 1;
    *ret_code = DN_PARSE_NAME_OKAY;
    *name_len = len;

    return p;
}

char *parse_question(char *current, char *orig_msg, dns_msg_q_t *q, int *ret_code) {
    char *p = current;
    int pn_code = DN_PARSE_NAME_INVALID;
    p = parse_name(p, orig_msg, q->name.labels, &q->name.len, &pn_code);
    if (pn_code == DN_PARSE_NAME_INVALID) {
        *ret_code = DNS_MSG_PARSE_Q_INVALID;
        return 0;
    }
    q->type = ntohs(*(uint16_t *)p);
    p += 2;
    q->class = ntohs(*(uint16_t *)p);
    p += 2;
    *ret_code = DNS_MSG_PARSE_Q_OKAY;
    return p;
}

int parse_dns_msg(char *msg, dns_msg_t *res) {
    char *p = msg;
    p = parse_header(msg, &res->header);
    dns_size_t qcount = res->header.qd_cnt;

    for (dns_size_t i = 0; i < qcount; i++) {
        int pq_code = DNS_MSG_PARSE_Q_INVALID;
        p = parse_question(p, msg, res->question + i, &pq_code);
        if (pq_code == DNS_MSG_PARSE_Q_INVALID) {
            return DNS_MSG_PARSE_INVALID;
        }
    }
    for (dns_size_t i = 0; i < res->header.an_cnt; i++) {
        int pr_code = DNS_MSG_PARSE_RR_OKAY;
        p = parse_rr(p, msg, res->answer + i, &pr_code);
        if (pr_code == DNS_MSG_PARSE_RR_INVALID) {
            return DNS_MSG_PARSE_INVALID;
        }
    }
    for (dns_size_t i = 0; i < res->header.ns_cnt; i++) {
        int pr_code = DNS_MSG_PARSE_RR_OKAY;
        p = parse_rr(p, msg, res->authority + i, &pr_code);
        if (pr_code == DNS_MSG_PARSE_RR_INVALID) {
            return DNS_MSG_PARSE_INVALID;
        }
    }
    for (dns_size_t i = 0; i < res->header.ar_cnt; i++) {
        int pr_code = DNS_MSG_PARSE_RR_OKAY;
        p = parse_rr(p, msg, res->additional + i, &pr_code);
        if (pr_code == DNS_MSG_PARSE_RR_INVALID) {
            return DNS_MSG_PARSE_INVALID;
        }
    }
    res->msg_len = p - msg;

    return DNS_MSG_PARSE_OKAY;
}

char *parse_rr(char *current, char *orig_msg, dns_msg_rr_t *rr, int *ret_code) {
    char *p = current;
    int code = 0;
    p = parse_name(p, orig_msg, rr->name.labels, &rr->name.len, &code);
    if (code == DNS_MSG_PARSE_Q_INVALID) {
        *ret_code = DNS_MSG_PARSE_RR_INVALID;
        return 0;
    }
    rr->type = ntohs(*(uint16_t *) p);
    p += 2;
    rr->class = ntohs(*(uint16_t *) p);
    p += 2;
    rr->ttl = ntohl(*(uint32_t *) p);
    p += 4;
    rr->rdlength = ntohs(*(uint16_t *) p);
    p += 2;
    rr->data_offset = p - orig_msg;
    p += rr->rdlength;
    *ret_code = DNS_MSG_PARSE_RR_OKAY;
    return p;
}
