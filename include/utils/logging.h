//
// Created by Yichen Xu on 2020/8/15.
//

#ifndef DNS_RELAY_LOGGING_H
#define DNS_RELAY_LOGGING_H

#include <stdio.h>

extern char _logging_mask;

void logging_init(char mask);

#define logd(args...) if (_logging_mask & 0x1) {fprintf(stdout, "[   debug ] "); fprintf(stderr, args); fprintf(stdout, "\n");}
#define logw(args...) if (_logging_mask & 0x2) {fprintf(stdout, "[ warning ] "); fprintf(stdout, args); fprintf(stdout, "\n");}
#define loge(args...) if (_logging_mask & 0x4) {fprintf(stderr, "[   error ] "); fprintf(stderr, args); fprintf(stdout, "\n");}
#define logi(args...) if (_logging_mask & 0x8) {fprintf(stdout, "[    info ] "); fprintf(stdout, args); fprintf(stdout, "\n");}

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

//void print_dns_msg(const char *data, uint32_t len) {
//    for (uint32_t i = 0; i < len; i++) {
//        printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(data[i]));
//        if (i % 2 == 1) {
//            putchar('\n');
//        }
//    }
//    putchar('\n');
//}

#endif //DNS_RELAY_LOGGING_H
