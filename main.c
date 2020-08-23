#include <stdio.h>
#include <socket.h>
#include <dns/parse.h>
#include <dns/utils.h>
#include <dns/compose.h>
#include <uv.h>
#include <db/io.h>
#include <utils/error.h>
#include <db/tree.h>
#include <db/cache.h>
#include <curl/curl.h>
#include <argparse.h>
#include <utils/logging.h>
#include <network/query_pool.h>
#include <network/conn_pool.h>
#include <network/udp_pool.h>
#include <ctype.h>

static void add_doh_connection(struct sockaddr addr, char *req_data, size_t req_len);
static void add_udp_req(struct sockaddr addr, char *req_data, size_t req_len);

uv_loop_t *loop;
uv_udp_t *srv_sock, *cli_sock;
uv_timer_t timeout;
struct sockaddr remote_addr;

CURLM *curl_handle;

ap_t *ap;
query_pool_t *qpool;
conn_pool_t *cpool;
udp_pool_t *upool;

tree_t *db_tree;
cache_t *db_cache;

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

static void on_send(uv_udp_send_t* handle, int status) {
    free(handle);
    if (status) {
        printf("uv_udp_send_cb error: %s\n", uv_strerror(status));
    }
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

        // send to client
        uv_udp_send_t *send_req = malloc(sizeof(uv_udp_send_t));
        uv_buf_t send_buf = uv_buf_init(buf->base, nread);
        uv_udp_send(send_req, srv_sock, &send_buf, 1, &qpool->pool[c->query_id].addr, on_send);
        upool_finish(upool, u_id);

        free(buf->base);
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
        logi("DNS message id %d, qcount %d", parsed_msg.header.id, parsed_msg.header.qd_cnt);
        printf("          ");
        print_question(parsed_msg.question, parsed_msg.raw);
        printf("\n");

        int hit = 0;
        if (parsed_msg.header.qd_cnt == 1 && parsed_msg.question[0].type == DNS_QTYPE_A) {
            // hosts lookup
            dn_db_name_t *name = db_name_from_dns_name(&parsed_msg.question[0].name, parsed_msg.raw);
            dn_db_record_t *rec = 0;
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
                    logw("Hit invalid address 0.0.0.0, ignore request");
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
            destroy_name(name);
        }

        // relay to dns server
        if (!hit && ap_get_int(ap, "doh_proxy")) {
            logi("request handler: doh server %s", ap_get_str(ap, "doh_server"));
            add_doh_connection(*addr, buf->base, nread);
        } else if (!hit) {
            logi("request handler: raw server %s", ap_get_str(ap, "server"));
            add_udp_req(*addr, buf->base, nread);
        }

        free(buf->base);
    }
}

typedef struct curl_context_s {
    uv_poll_t poll_handle;
    curl_socket_t sockfd;
} curl_context_t;

static curl_context_t *create_curl_context(curl_socket_t sockfd)
{
    curl_context_t *context;

    context = (curl_context_t *) malloc(sizeof(*context));

    context->sockfd = sockfd;

    uv_poll_init_socket(loop, &context->poll_handle, sockfd);
    context->poll_handle.data = context;

    return context;
}

static void curl_close_cb(uv_handle_t *handle)
{
    curl_context_t *context = (curl_context_t *) handle->data;
    free(context);
}

static void destroy_curl_context(curl_context_t *context)
{
    uv_close((uv_handle_t *) &context->poll_handle, curl_close_cb);
}

static size_t handle_data(char *buf, size_t size, size_t nmemb, void *userdata) {
    conn_context_t *context = (conn_context_t *) userdata;
    size_t realsize = size * nmemb;

    memcpy(&context->read_buf[context->nread], buf, realsize);
    context->nread += realsize;

    return realsize;
}

static void add_udp_req(struct sockaddr addr, char *req_data, size_t req_len) {
    // check whether pool is full
    if (qpool_full(qpool)) {
        logw("ignore dns request due to full query pool.");
        return;
    }
    if (upool_full(upool)) {
        logw("ignore dns request due to full udp req pool.");
        return;
    }
    int q_id = qpool_insert(qpool, addr, req_data, req_len);
    // get udp context id
    int u_id = upool_add(upool, q_id, qpool->pool + q_id);

    uv_udp_send_t *send_req = malloc(sizeof(uv_udp_send_t));
    uv_buf_t send_buf = uv_buf_init(upool->pool[u_id]->send_buf, upool->pool[u_id]->send_len);
    uv_udp_send(send_req, cli_sock, &send_buf, 1, &remote_addr, on_send);
}

static void add_doh_connection(struct sockaddr addr, char *req_data, size_t req_len) {
    // add to query pool
    if (qpool_full(qpool)) {
        logw("ignore dns request due to full query pool.");
        return;
    }
    if (cpool_full(cpool)) {
        logw("ignore dns request due to full doh conn pool.");
        return;
    }
    int q_id = qpool_insert(qpool, addr, req_data, req_len);
    // get curl handle
    conn_context_t *conn = cpool_add_conn(cpool, q_id, qpool->pool + q_id);

    CURL *easy_handle = conn->easy_handle;

    // construct the post request
    // prepare headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/dns-message");
    headers = curl_slist_append(headers, "Accept: application/dns-message");
    char s[1024];
    snprintf(s, 1024, "Content-Length: %ld", conn->send_len);
    headers = curl_slist_append(headers, s);
    // prepare body
    curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, conn->send_buf);
    curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDSIZE, conn->send_len);
    curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, headers);

    // set up write handler
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, handle_data);

    // verbose
    curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, ap_get_int(ap, "curl_verbose"));

    curl_multi_add_handle(curl_handle, easy_handle);
}

static void check_multi_info(void)
{
    char *done_url;
    CURLMsg *message;
    int pending;
    CURL *easy_handle;
    conn_context_t *context;
    int q_id;
    query_t *q;
    int c_id;

    while((message = curl_multi_info_read(curl_handle, &pending))) {
        switch(message->msg) {
            case CURLMSG_DONE:
                easy_handle = message->easy_handle;

                curl_easy_getinfo(easy_handle, CURLINFO_EFFECTIVE_URL, &done_url);
                curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &context);
                q_id = context->query_id;
                q = qpool->pool + q_id;
                c_id = context->conn_id;

                curl_multi_remove_handle(curl_handle, easy_handle);
                curl_easy_cleanup(easy_handle);
                if (context->nread) {
                    logi("receive response from doh server, length %ld", context->nread);
                    // change dns message id
                    set_dns_id(context->read_buf, q->dns_id);
                    uv_udp_send_t *send_req = malloc(sizeof(uv_udp_send_t));
                    uv_buf_t send_buf = uv_buf_init(context->read_buf, context->nread);
                    uv_udp_send(send_req, srv_sock, &send_buf, 1, &qpool->pool[context->query_id].addr, on_send);
                }
                qpool_remove(qpool, q_id);
                cpool_finish(cpool, c_id);
                break;

            default:
                logi("CURLMSG default");
                break;
        }
    }
}

static void curl_perform(uv_poll_t *req, int status, int events)
{
    int running_handles;
    int flags = 0;
    curl_context_t *context;

    if(events & UV_READABLE)
        flags |= CURL_CSELECT_IN;
    if(events & UV_WRITABLE)
        flags |= CURL_CSELECT_OUT;

    context = (curl_context_t *) req->data;

    curl_multi_socket_action(curl_handle, context->sockfd, flags,
                             &running_handles);

    check_multi_info();
}

static void on_timeout(uv_timer_t *req)
{
    int running_handles;
    curl_multi_socket_action(curl_handle, CURL_SOCKET_TIMEOUT, 0,
                             &running_handles);
    check_multi_info();
}

static int start_timeout(CURLM *multi, long timeout_ms, void *userp)
{
    if(timeout_ms < 0) {
        uv_timer_stop(&timeout);
    }
    else {
        if(timeout_ms == 0)
            timeout_ms = 1; /* 0 means directly call socket_action, but we'll do it
                         in a bit */
        uv_timer_start(&timeout, on_timeout, timeout_ms, 0);
    }
    return 0;
}

static int handle_socket(CURL *easy, curl_socket_t s, int action, void *userp,
                         void *socketp)
{
    curl_context_t *curl_context;
    int events = 0;

    switch(action) {
        case CURL_POLL_IN:
        case CURL_POLL_OUT:
        case CURL_POLL_INOUT:
            curl_context = socketp ?
                           (curl_context_t *) socketp : create_curl_context(s);

            curl_multi_assign(curl_handle, s, (void *) curl_context);

            if(action != CURL_POLL_IN)
                events |= UV_WRITABLE;
            if(action != CURL_POLL_OUT)
                events |= UV_READABLE;

            uv_poll_start(&curl_context->poll_handle, events, curl_perform);
            break;
        case CURL_POLL_REMOVE:
            if(socketp) {
                uv_poll_stop(&((curl_context_t*)socketp)->poll_handle);
                destroy_curl_context((curl_context_t*) socketp);
                curl_multi_assign(curl_handle, s, NULL);
            }
            break;
        default:
            abort();
    }

    return 0;
}

int main(int argc, const char **argv) {
    logi("dns relay is starting");
    // parse arguments
    ap = ap_init();
    ap_add_argument(ap, "server", AP_STR, "1.1.1.1");
    ap_add_argument(ap, "doh_server", AP_STR, "cloudflare-dns.com");
    ap_add_argument(ap, "doh_proxy", AP_STORE_TRUE, NULL);
    int max_query = 32;
    ap_add_argument(ap, "max_query", AP_INT, &max_query);
    int max_doh_conn = 32;
    ap_add_argument(ap, "max_doh_conn", AP_INT, &max_doh_conn);
    int max_udp_req = 32;
    ap_add_argument(ap, "max_udp_req", AP_INT, &max_udp_req);
    ap_add_argument(ap, "curl_verbose", AP_STORE_TRUE, NULL);
    ap_parse(ap, argc, argv);
    logi("remote server: %s", ap_get_str(ap, "server"));
    logi("remote DoH server: %s", ap_get_str(ap, "doh_server"));
    if (ap_get_int(ap, "doh_proxy")) {
        logi("DoH proxy is enabled");
    } else {
        logi("DoH proxy is not enabled");
    }

    loop = uv_default_loop();
    srv_sock = malloc(sizeof(uv_udp_t));
    cli_sock = malloc(sizeof(uv_udp_t));

    qpool = qpool_init(ap_get_int(ap, "max_query"));
    cpool = cpool_init(ap_get_int(ap, "max_doh_conn"), ap_get_str(ap, "doh_server"));
    upool = upool_init(ap_get_int(ap, "max_udp_req"));
    logi("query pool and doh conn pool initialized, max query %d, max doh conn %d",
         ap_get_int(ap, "max_query"),
         ap_get_int(ap, "max_doh_conn"));

    // initialize addr
    struct sockaddr_in srv_addr;
    struct sockaddr_in cli_addr;
    uv_ip4_addr("0.0.0.0", 53, &srv_addr);
    uv_ip4_addr("0.0.0.0", 2345, &cli_addr);
    uv_ip4_addr(ap_get_str(ap, "server"), 53, (struct sockaddr_in *) &remote_addr);

    // initialize server socket
    uv_udp_init(loop, srv_sock);
    uv_udp_bind(srv_sock, (const struct sockaddr *)&srv_addr, UV_UDP_REUSEADDR);
    uv_udp_recv_start(srv_sock, alloc_buffer, on_srv_read);

    // initialize client socket
    uv_udp_init(loop, cli_sock);
    uv_udp_bind(cli_sock, (const struct sockaddr *)&cli_addr, UV_UDP_REUSEADDR);
    uv_udp_recv_start(cli_sock, alloc_buffer, on_cli_read);

    // initialize curl
    if (curl_global_init(CURL_GLOBAL_ALL)) {
        loge("could not init curl");
        error("fatal error: can not initialize curl globally");
    }

    // load from hosts.txt
    int count = 0;
    int db_ret_code = 0;
    dn_db_record_t *db_rec = db_read_all_records("/Users/linyxus/dev/dns-relay/hosts.txt", &count, &db_ret_code);
    if (db_ret_code != DB_PARSE_RECORD_EOF) {
        loge("Error parsing hosts file.");
        error("Fatal error when starting server.");
    }
    db_tree = tree_build_from_rec(db_rec, count);
    db_cache = cache_init();
    logi("Successfully initialized host database.");

    uv_timer_init(loop, &timeout);

    curl_handle = curl_multi_init();
    curl_multi_setopt(curl_handle, CURLMOPT_SOCKETFUNCTION, handle_socket);
    curl_multi_setopt(curl_handle, CURLMOPT_TIMERFUNCTION, start_timeout);

    logi("server listening at port 53");
    logi("dns client running at port 2345");
    int ret_code = uv_run(loop, UV_RUN_DEFAULT);

    qpool_free(qpool);
    cpool_free(cpool);
    upool_free(upool);
    curl_multi_cleanup(curl_handle);
    ap_free(ap);

    return ret_code;
}
