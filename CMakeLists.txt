cmake_minimum_required(VERSION 3.16)
project(dns_relay C)

set(CMAKE_C_STANDARD 99)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_library(libdns_relay
        includes/db/io.h
        includes/db/parse.h
        includes/db/types.h
        src/db/dn.c
        src/db/parse.c
        includes/dns/parse.h
        includes/dns/types.h
        includes/dns/utils.h
        src/dns/error.c
        src/dns/parse.c
        includes/utils/error.h
        includes/utils/queue.h
        includes/utils/total_map.h
        src/utils/err.c
        src/utils/queue.c
        src/utils/total_map.c
        includes/utils/argparse.h
        includes/utils/logging.h
        includes/network/query_pool.h
        src/utils/argparse.c
        src/network/query_pool.c
        includes/network/conn_pool.h src/network/conn_pool.c src/db/io.c includes/db/tree.h src/db/tree.c includes/dns/compose.h src/dns/compose.c
        includes/db/cache.h src/db/cache.c includes/network/udp_pool.h src/network/udp_pool.c)

target_include_directories(
        libdns_relay
        PUBLIC
        "includes"
)
target_link_libraries(libdns_relay CONAN_PKG::libuv)
target_link_libraries(libdns_relay CONAN_PKG::libcurl)

add_executable(dns_relay src/main.c)

include_directories(test/include)
add_executable(ctest test/ctest.c test/src/cmocka.c)

target_link_libraries(dns_relay CONAN_PKG::libuv)
target_link_libraries(dns_relay CONAN_PKG::libcurl)
target_link_libraries(dns_relay libdns_relay)
target_link_libraries(ctest libdns_relay)
