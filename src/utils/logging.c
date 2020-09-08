//
// Created by Yichen Xu on 2020/9/8.
//

#include <utils/logging.h>

char _logging_mask = 0xf;

void logging_init(char level) {
    _logging_mask = level;
}
