#include <utils/argparse.h>
#include <utils/logging.h>
#include <server/dns_server.h>
#include <unistd.h>

static ap_t *ap = 0;
static char cwd_buf[FILENAME_MAX];

int main(int argc, const char **argv) {
    getcwd(cwd_buf, FILENAME_MAX);
    // parse arguments
    ap = ap_init();
    ap_add_argument(ap, "server", AP_STR, "Raw remote DNS server address (default 1.1.1.1).", "1.1.1.1");
    ap_add_argument(ap, "doh_server", AP_STR, "Remote DoH server address (default cloudflare-dns.com).", "cloudflare-dns.com");
    int doh_proxy = 0;
    ap_add_argument(ap, "doh_proxy", AP_STORE_TRUE, "Enable DoH proxy mode.", &doh_proxy);
    int max_query = 32;
    ap_add_argument(ap, "max_query", AP_INT, "Max concurrent query number (default 32).", &max_query);
    int max_doh_conn = 32;
    ap_add_argument(ap, "max_doh_conn", AP_INT, "Max concurrent DoH connection number (default 32).", &max_doh_conn);
    int max_udp_req = 32;
    ap_add_argument(ap, "max_udp_req", AP_INT, "Max concurrent UDP request number (default 32).", &max_udp_req);
    ap_add_argument(ap, "curl_verbose", AP_STORE_TRUE, "Enable verbose curl debug output.", NULL);
    int client_port = 2345;
    ap_add_argument(ap, "client_port", AP_INT, "Client UDP socket port for raw DNS request (default 2345).", &client_port);
    ap_add_argument(ap, "hosts_path", AP_STR, "Path to hosts file.", "/Users/linyxus/dev/dns-relay/hosts.txt");
    int mask = 0xf;
    ap_add_argument(ap, "logging", AP_INT, "Logging mask for controlling output verbosity (default 15).", &mask);
    ap_parse(ap, argc, argv);

    logging_init(ap_get_int(ap, "logging"));

    logi("dns relay is starting");

    // print basic info
    if (ap_get_int(ap, "doh_proxy")) {
        logi("running as DNS over HTTPS proxy");
        logi("remote DoH server: %s", ap_get_str(ap, "doh_server"));
    } else {
        logi("running as raw DNS relay proxy");
        logi("remote server: %s", ap_get_str(ap, "server"));
    }
    logi("curl verbose: %d, max query: %d, max doh conns: %d, max udp req: %d, client port: %d, hosts file: %s, working dir: %s",
         ap_get_int(ap, "curl_verbose"),
         ap_get_int(ap, "max_query"),
         ap_get_int(ap, "max_doh_conn"),
         ap_get_int(ap, "max_udp_req"),
         ap_get_int(ap, "client_port"),
         ap_get_str(ap, "hosts_path"),
         cwd_buf
         );

    // init server
    dns_server_init(ap);
    logi("server initialized, listening at port %d", 53);

    // run server
    int ret_code = dns_server_run();

    // free up resources
    dns_server_deinit();
    ap_free(ap);

    return ret_code;
}
