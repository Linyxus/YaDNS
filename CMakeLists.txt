cmake_minimum_required(VERSION 3.16)
project(dns_relay C)

set(CMAKE_C_STANDARD 99)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_library(libdns_relay
        include/db/io.h
        include/db/parse.h
        include/db/types.h
        src/db/dn.c
        src/db/parse.c
        include/dns/parse.h
        include/dns/types.h
        include/dns/utils.h
        src/dns/error.c
        src/dns/parse.c
        include/utils/error.h
        include/utils/queue.h
        include/utils/total_map.h
        src/utils/err.c
        src/utils/queue.c
        src/utils/total_map.c
        include/utils/argparse.h
        include/utils/logging.h
        include/network/query_pool.h
        src/utils/argparse.c
        src/network/query_pool.c
        include/network/conn_pool.h src/network/conn_pool.c src/db/io.c include/db/tree.h src/db/tree.c include/dns/compose.h src/dns/compose.c
        include/db/cache.h src/db/cache.c include/network/udp_pool.h src/network/udp_pool.c include/server/pools.h src/server/pools.c include/server/loop.h src/server/loop.c include/server/socket.h src/server/socket.c include/server/db_cache.h src/server/db_cache.c include/server/curl.h src/server/curl.c include/server/dns_server.h src/server/dns_server.c)

target_include_directories(
        libdns_relay
        PUBLIC
        "include"
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
