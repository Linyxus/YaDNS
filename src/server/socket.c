//
// Created by Yichen Xu on 2020/8/23.
//

#include <server/socket.h>
#include <server/loop.h>
#include <server/pools.h>
#include <server/db_cache.h>
#include <server/curl.h>
#include <db/io.h>
#include <db/lru_cache.h>
#include <dns/utils.h>
#include <dns/compose.h>
#include <dns/print.h>
#include <stdlib.h>
#include <ctype.h>
#include <utils/logging.h>

uv_udp_t *srv_sock = 0;
uv_udp_t *cli_sock = 0;
static struct sockaddr remote_addr;

static ap_t *ap;

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

void on_send(uv_udp_send_t* handle, int status) {
    free(handle);
    if (status) {
        printf("uv_udp_send_cb error: %s\n", uv_strerror(status));
    }
}

static dn_db_name_t *db_name_from_dns_name(dn_name_t *dns_name, char *raw) {
    dn_db_name_t *name = 0;
    for (size_t i = 0; i < dns_name->len; i++) {
        dn_db_name_t *u = malloc(sizeof(dn_db_name_t));
        u->label.len = dns_name->labels[i].len;
        u->label.label = malloc(sizeof(char) * (u->label.len + 1));
        memcpy(u->label.label, raw + dns_name->labels[i].offset, u->label.len);
        // convert to lower case
        for (size_t j = 0; j < u->label.len; j++) {
            u->label.label[j] = tolower(u->label.label[j]);
        }
        u->label.label[u->label.len] = '\0';
        u->next = name;
        name = u;
    }

    return name;
}


static void on_cli_read(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf,
                        const struct sockaddr *addr, unsigned flags) {
    if (nread < 0) {
        printf("recv error: %s\n", uv_err_name(nread));
        uv_close((uv_handle_t *)handle, NULL);
        free(buf->base);
        return;
    }

    if (nread > 0) {
        char sender[17] = { 0 };
        uint16_t port = ntohs(*(uint16_t *) addr->sa_data);
        uv_ip4_name((const struct sockaddr_in *)addr, sender, 16);
        logi("recv server response from %s:%d", sender, port);

        // find corresponding query
        uint16_t dns_id = get_dns_id(buf->base);
        int u_id = -1;
        for (size_t i = 0; i < upool->pool_size; i++) {
            if (dns_id == upool->pool[i]->dns_id && upool->pool[i]->valid) {
                u_id = i;
                break;
            }
        }
        if (u_id == -1) {
            logw("unrecognized dns id in response");
            free(buf->base);
            return;
        }
        udp_context_t *c = upool->pool[u_id];
        set_dns_id(buf->base, qpool->pool[c->query_id].dns_id);

        uv_timer_t *timer = c->timer;
        uv_timer_stop(timer);
        free(timer->data);
        free(timer);

        // send to client
        uv_udp_send_t *send_req = malloc(sizeof(uv_udp_send_t));
        uv_buf_t send_buf = uv_buf_init(buf->base, nread);
        uv_udp_send(send_req, srv_sock, &send_buf, 1, &qpool->pool[c->query_id].addr, on_send);
        upool_finish(upool, u_id);
        qpool_remove(qpool, c->query_id);

        // parse the buffer
        dns_msg_t msg;
        int ret_code = parse_dns_msg(buf->base, &msg);
        memcpy(msg.raw, buf->base, msg.msg_len);
        if (ret_code != DNS_MSG_PARSE_OKAY) {
            loge("fail to parse dns message");
        } else {
            print_dns_msg(&msg);

            for (int i = 0; i < msg.header.an_cnt; i++) {
                dns_msg_rr_t *rr = msg.answer + i;
                if (rr->type == 1) {
                    dn_db_name_t *name = db_name_from_dns_name(&rr->name, msg.raw);
                    db_ip_t ip = ntohl(*(uint32_t *) (rr->data_offset + msg.raw));
                    uint32_t ttl = rr->ttl;
//                    uint32_t ttl = 120;

                    lc_insert(db_lru_cache, name, ip, ttl);
                }
            }

            for (int i = 0; i < msg.header.ns_cnt; i++) {
                dns_msg_rr_t *rr = msg.authority + i;
                if (rr->type == 1) {
                    dn_db_name_t *name = db_name_from_dns_name(&rr->name, msg.raw);
                    db_ip_t ip = ntohl(*(uint32_t *) (rr->data_offset + msg.raw));
                    uint32_t ttl = rr->ttl;

                    lc_insert(db_lru_cache, name, ip, ttl);
                }
            }
        }

        free(buf->base);
    }
}

static void on_udp_timeout(uv_timer_t *timer) {
    logw("udp %d timeout, delete req", *(int *)timer->data);
    int u_id = *(int *) timer->data;
    uv_timer_stop(timer);
    free(timer->data);
    free(timer);
    udp_context_t *c = upool->pool[u_id];
    upool_finish(upool, u_id);
    qpool_remove(qpool, c->query_id);
}

static void add_udp_req(struct sockaddr addr, char *req_data, size_t req_len, dn_db_name_t *dn_name) {
    // check whether pool is full
    if (qpool_full(qpool)) {
        logw("ignore dns request due to full query pool.");
        return;
    }
    if (upool_full(upool)) {
        logw("ignore dns request due to full udp req pool.");
        return;
    }
    int q_id = qpool_insert(qpool, addr, req_data, req_len, dn_name);
    // get udp context id
    int u_id = upool_add(upool, q_id, qpool->pool + q_id);

    uv_timer_t *timer = malloc(sizeof(uv_timer_t));
    uv_timer_init(loop, timer);
    int *p_uid = malloc(sizeof(int));
    *p_uid = u_id;
    timer->data = p_uid;
    uv_timer_start(timer, on_udp_timeout, UDP_TIMEOUT, UDP_TIMEOUT);

    upool->pool[u_id]->timer = timer;

    uv_udp_send_t *send_req = malloc(sizeof(uv_udp_send_t));
    uv_buf_t send_buf = uv_buf_init(upool->pool[u_id]->send_buf, upool->pool[u_id]->send_len);
    uv_udp_send(send_req, cli_sock, &send_buf, 1, &remote_addr, on_send);
}

static void on_srv_read(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf,
                        const struct sockaddr *addr, unsigned flags) {
    if (nread < 0) {
        printf("recv error: %s\n", uv_err_name(nread));
        uv_close((uv_handle_t *)handle, NULL);
        free(buf->base);
        return;
    }

    if (nread > 0) {
        char sender[17] = { 0 };
        uint16_t port = ntohs(*(uint16_t *) addr->sa_data);
        uv_ip4_name((const struct sockaddr_in *)addr, sender, 16);
        logi("recv client request from %s:%d, length %ld", sender, port, nread);

        dns_msg_t parsed_msg;
        int ret_code = parse_dns_msg(buf->base, &parsed_msg);
        memcpy(parsed_msg.raw, buf->base, nread);
        if (ret_code != DNS_MSG_PARSE_OKAY) {
            loge("Error parsing dns message");
        }
//        logi("DNS message id %d, qcount %d", parsed_msg.header.id, parsed_msg.header.qd_cnt);
        print_dns_msg(&parsed_msg);

        int hit = 0;
        dn_db_name_t *name = 0;
        dn_db_record_t *rec = 0;
        // first check hosts cache
        if (parsed_msg.header.qd_cnt == 1 && parsed_msg.question[0].type == DNS_QTYPE_A) {
            // hosts lookup
            name = db_name_from_dns_name(&parsed_msg.question[0].name, parsed_msg.raw);
            rec = cache_lookup(db_cache, name);
            if (!rec) {
                logi("hosts cache miss");
                rec = tree_lookup(db_tree, name);
                if (rec) {
                    logi("hosts cache refilled");
                    cache_insert(db_cache, name, rec);
                }
            } else {
                logi("hosts cache hit");
            }
            char *rr;
            char *reply;
            if (rec) {
                logi("request handler: hosts");
                hit = 1;
                if (!rec->ip) {
                    logw("Hit invalid address 0.0.0.0, return nxdomain");
                    dns_msg_header_t h = parsed_msg.header;
                    h.qr = 1;
                    h.ar_cnt = 0;
                    h.qd_cnt = 0;
                    h.ns_cnt = 0;
                    h.an_cnt = 0;
                    h.rcode = 3;

                    size_t reply_len;
                    reply = compose_header(&h, &reply_len);
                    uv_udp_send_t *send_req = malloc(sizeof(uv_udp_send_t));
                    uv_buf_t send_buf = uv_buf_init(reply, reply_len);
                    uv_udp_send(send_req, srv_sock, &send_buf, 1, addr, on_send);
                    free(reply);
                } else {
                    // compose rr record
                    size_t rr_len;
                    rr = compose_a_rr(&parsed_msg.question[0].name, rec->ip, &rr_len);
                    size_t reply_len;
                    reply = compose_a_rr_ans(parsed_msg.raw, parsed_msg.msg_len, rr, rr_len, &reply_len);
                    uv_udp_send_t *send_req = malloc(sizeof(uv_udp_send_t));
                    uv_buf_t send_buf = uv_buf_init(reply, reply_len);
                    uv_udp_send(send_req, srv_sock, &send_buf, 1, addr, on_send);
                    free(rr);
                    free(reply);
                }
            }
            // destroy_name(name); // do not destroy name right now. Saved for further use
        }
        // then check lru cache
        if (parsed_msg.header.qd_cnt == 1 && parsed_msg.question[0].type == DNS_QTYPE_A) {
            name = db_name_from_dns_name(&parsed_msg.question[0].name, parsed_msg.raw);
            db_ip_t ip = lc_lookup(db_lru_cache, name);
            if (ip) {
                logi("request handler: local lru cache");
                char *rr;
                char *reply;
                hit = 1;
                // compose rr record
                size_t rr_len;
                rr = compose_a_rr(&parsed_msg.question[0].name, ip, &rr_len);
                size_t reply_len;
                reply = compose_a_rr_ans(parsed_msg.raw, parsed_msg.msg_len, rr, rr_len, &reply_len);
                uv_udp_send_t *send_req = malloc(sizeof(uv_udp_send_t));
                uv_buf_t send_buf = uv_buf_init(reply, reply_len);
                uv_udp_send(send_req, srv_sock, &send_buf, 1, addr, on_send);
                free(rr);
                free(reply);
            }
        }

        // relay to dns server
        if (!hit && ap_get_int(ap, "doh_proxy")) {
            logi("request handler: doh server %s", ap_get_str(ap, "doh_server"));
            add_doh_connection(*addr, buf->base, nread, name);
        } else if (!hit) {
            logi("request handler: raw server %s", ap_get_str(ap, "server"));
            add_udp_req(*addr, buf->base, nread, name);
        }

        free(buf->base);
    }
}

void sock_init(ap_t *args) {
    ap = args;
    srv_sock = malloc(sizeof(uv_udp_t));
    cli_sock = malloc(sizeof(uv_udp_t));

    struct sockaddr_in srv_addr;
    struct sockaddr_in cli_addr;
    uv_ip4_addr("0.0.0.0", 53, &srv_addr);
    uv_ip4_addr("0.0.0.0", ap_get_int(ap, "client_port"), &cli_addr);
    uv_ip4_addr(ap_get_str(ap, "server"), 53, (struct sockaddr_in *) &remote_addr);

    // initialize server socket
    uv_udp_init(loop, srv_sock);
    uv_udp_bind(srv_sock, (const struct sockaddr *)&srv_addr, UV_UDP_REUSEADDR);
    uv_udp_recv_start(srv_sock, alloc_buffer, on_srv_read);

    // initialize client socket
    uv_udp_init(loop, cli_sock);
    uv_udp_bind(cli_sock, (const struct sockaddr *)&cli_addr, UV_UDP_REUSEADDR);
    uv_udp_recv_start(cli_sock, alloc_buffer, on_cli_read);
}
