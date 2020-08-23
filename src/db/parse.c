//
// Created by Yichen Xu on 2020/6/25.
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <db/parse.h>
#include <db/types.h>

db_ip_t _db_parse_ip(const char *str, uint16_t len, int *ret_code, db_ip_t acc, int depth);

dn_db_name_t *db_parse_name(const char *str, uint16_t len, int *ret_code) {
    if (!len) {
        *ret_code = DB_PARSE_NAME_OKAY;
        return 0;
    }

    dn_db_name_t *ret = malloc(sizeof(dn_db_name_t));
    char *p = str;
    while (*p != '.' && p != str + len) p++;
    int label_len = p - str;

    if (!label_len) {
        free(ret);
        *ret_code = DB_PARSE_NAME_INVALID;
        return 0;
    }

    ret->label.label = malloc(sizeof(char) * (label_len + 1));
    memcpy(ret->label.label, str, label_len);
    ret->label.label[label_len] = 0;
    ret->label.len = label_len;

    len = len - label_len;
    if (*p == '.') {
        p++; len--;
        if (!len) {
            *ret_code = DB_PARSE_NAME_INVALID;
            return 0;
        }
    }

    int code = 0;
    dn_db_name_t *rest = db_parse_name(p, len, &code);
    if (code == DB_PARSE_NAME_INVALID) {
        *ret_code = code;
        return 0;
    }

    *ret_code = DB_PARSE_NAME_OKAY;
    ret->next = 0;
    for (int i = 0; ret->label.label[i]; i++) {
        ret->label.label[i] = tolower(ret->label.label[i]);
    }
    if (rest) {
        dn_db_name_t *x = rest;
        while (x->next) x = x->next;
        x->next = ret;

        return rest;
    } else {
        return ret;
    }
}

db_ip_t db_parse_ip(const char *str, uint16_t len, int *ret_code) {
    return _db_parse_ip(str, len, ret_code, 0, 3);
}

/**
 * 从字符串中读出一个整数，要求字符串中只包含数字，否则会带来无法预料的结果
 * @param str 输入字符串
 * @param len 字符串长度
 * @return 读取到的整数值
 */
uint16_t readint(const char *str, uint16_t len) {
    uint16_t ret = 0;
    for (const char *p = str; p != str + len; p++) {
        ret = ret * 10 + (*p) - '0';
    }
    return ret;
}

db_ip_t _db_parse_ip(const char *str, uint16_t len, int *ret_code, db_ip_t acc, int depth) {
    const char *p = str;
    while (*p != '.' && p != str + len) {
        if (!isdigit(*p)) {
            *ret_code = DB_PARSE_IP_INVALID;
            return 0;
        }
        p++;
    }

    if (depth == 0) {
        if (p != str + len || len == 0) {
            *ret_code = DB_PARSE_IP_INVALID;
            return 0;
        }
        *ret_code = DB_PARSE_IP_OKAY;
        return acc * 256 + readint(str, len);
    }

    if (*p != '.') {
        *ret_code = DB_PARSE_IP_INVALID;
        return 0;
    }

    uint16_t slen = p - str;
    if (slen == 0) {
        *ret_code = DB_PARSE_IP_INVALID;
        return 0;
    }
    p++; len--;
    len -= slen;
    db_ip_t x = readint(str, slen);
    return _db_parse_ip(p, len, ret_code, acc * 256 + x, depth - 1);
}

/**
 * 试图从输入字符串中提取出一条记录 (<ip> <domain name>)
 * @param str 指向输入字符串的指针的指针，若成功读取一条记录，会将指针移动到记录结束的地方
 * @param end 字符串结尾的指针
 * @param res 保存结果的指针
 * @return 返回状态值
 *  0 OKAY
 *  1 INVALID
 *  2 EOF (读取到了字符串的末尾)
 */
int db_parse_next_record(char **str, char *end, dn_db_record_t *res) {
    char *p = *str;
    while ((*p == '\n' || *p == '\r') && p != end) p++;
    if (p == end) {
        *str = end;
        return DB_PARSE_RECORD_EOF;
    }

    char *q = p;
    while (*q != '\n' && *q != '\r' && q != end) q++;

    char *m = p;
    while (*m != ' ' && m != end) m++;
    if (m == end || m == end - 1) {
        *str = end;
        return DB_PARSE_RECORD_INVALID;
    }

    int code = -1;
    res->ip = db_parse_ip(p, m - p, &code);
    if (code == DB_PARSE_IP_INVALID) return DB_PARSE_RECORD_INVALID;

    res->name = db_parse_name(m + 1, q - m - 1, &code);
    if (code == DB_PARSE_NAME_INVALID) return DB_PARSE_RECORD_INVALID;

    *str = q;

    return DB_PARSE_RECORD_OKAY;
}
