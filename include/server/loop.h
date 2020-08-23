//
// Created by Yichen Xu on 2020/8/23.
//

#ifndef DNS_RELAY_LOOP_H
#define DNS_RELAY_LOOP_H

#include <uv.h>

extern uv_loop_t *loop;

void loop_init();

#endif //DNS_RELAY_LOOP_H
