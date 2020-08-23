//
// Created by Yichen Xu on 2020/8/10.
//

#include "utils/error.h"
#include <stdlib.h>
#include <stdio.h>

void error(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(-1);
}
