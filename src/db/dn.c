//
// Created by Yichen Xu on 2020/6/25.
//

#include <stdlib.h>
#include <db/types.h>

void destroy_name(dn_db_name_t *p) {
    if (!p) return;
    destroy_name(p->next);
    free(p);
    p = 0;
}
