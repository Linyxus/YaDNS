//
// Created by Yichen Xu on 2020/8/23.
//

#include <server/loop.h>

uv_loop_t *loop = 0;

void loop_init() {
    loop = uv_default_loop();
}
