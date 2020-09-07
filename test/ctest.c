//
// Created by Yichen Xu on 2020/6/25.
//

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <db/types.h>
#include <db/parse.h>
#include <utils/queue.h>
#include <string.h>
#include <utils/trie.h>

void test_always_success(void **state) {
    // This test will always pass
}

void test_db_destroy_name_null_return_null(void **state) {
    dn_db_name_t *p = 0;
    destroy_name(p);
    assert_null(p);
}

void test_db_parse_name_correct_1(void **state) {
    int code = -1;
    dn_db_name_t *name = db_parse_name("www.baidu.com", 13, &code);
    dn_db_name_t *p = name;

    assert_non_null(p);
    assert_string_equal("com", p->label.label);
    assert_int_equal(3, p->label.len);

    p = p->next;
    assert_non_null(p);
    assert_string_equal("baidu", p->label.label);
    assert_int_equal(5, p->label.len);

    p = p->next;
    assert_non_null(p);
    assert_string_equal("www", p->label.label);
    assert_int_equal(3, p->label.len);

    p = p->next;
    assert_null(p);
}

void test_db_parse_name_correct_2(void **state) {
    int code = -1;
    dn_db_name_t *name = db_parse_name("www.BAIDU.c0m", 13, &code);
    dn_db_name_t *p = name;

    assert_non_null(p);
    assert_int_equal(code, DB_PARSE_NAME_OKAY);
    assert_string_equal("c0m", p->label.label);
    assert_int_equal(3, p->label.len);

    p = p->next;
    assert_non_null(p);
    assert_string_equal("baidu", p->label.label);
    assert_int_equal(5, p->label.len);

    p = p->next;
    assert_non_null(p);
    assert_string_equal("www", p->label.label);
    assert_int_equal(3, p->label.len);

    p = p->next;
    assert_null(p);
}

void test_db_parse_name_null(void **state) {
    int code = -1;
    dn_db_name_t *name = db_parse_name("", 0, &code);

    assert_null(name);
    assert_int_equal(code, DB_PARSE_NAME_OKAY);
}

void test_db_parse_name_invalid(void **state) {
    int code = -1;
    dn_db_name_t *name = db_parse_name("...", 3, &code);
    assert_null(name);
    assert_int_equal(code, DB_PARSE_NAME_INVALID);

    name = db_parse_name("www.baidu.", 10, &code);
    assert_null(name);
    assert_int_equal(code, DB_PARSE_NAME_INVALID);
}

void test_db_parse_ip_correct_0(void **state) {
    int code = -1;

    char s0[] = "0.0.0.10";
    db_ip_t ip = db_parse_ip(s0, strlen(s0), &code);
    assert_int_equal(ip, 10);
    assert_int_equal(code, DB_PARSE_IP_OKAY);
}

void test_db_parse_ip_correct_1(void **state) {
    int code = -1;

    char s0[] = "1.1.1.1";
    db_ip_t ip = db_parse_ip(s0, strlen(s0), &code);
    assert_int_equal(ip, 16843009);
    assert_int_equal(code, DB_PARSE_IP_OKAY);
}

void test_db_parse_ip_correct_2(void **state) {
    int code = -1;

    char s0[] = "4.3.2.1";
    db_ip_t ip = db_parse_ip(s0, strlen(s0), &code);
    assert_int_equal(ip, 67305985);
    assert_int_equal(code, DB_PARSE_IP_OKAY);
}

void test_db_parse_ip_correct_3(void **state) {
    int code = -1;

    char s0[] = "10.0.0.000";
    db_ip_t ip = db_parse_ip(s0, strlen(s0), &code);
    assert_int_equal(ip, 167772160);
    assert_int_equal(code, DB_PARSE_IP_OKAY);
}

void test_db_parse_ip_invalid_end(void **state) {
    int code = -1;

    char s0[] = "0.0.0.";
    db_ip_t ip = db_parse_ip(s0, strlen(s0), &code);
    assert_int_equal(ip, 0);
    assert_int_equal(code, DB_PARSE_IP_INVALID);
}

void test_db_parse_ip_invalid_mid(void **state) {
    int code = -1;

    char s0[] = "0..10.1";
    db_ip_t ip = db_parse_ip(s0, strlen(s0), &code);
    assert_int_equal(ip, 0);
    assert_int_equal(code, DB_PARSE_IP_INVALID);
}

void test_db_parse_ip_invalid_non_digit(void **state) {
    int code = -1;

    char s0[] = "0.0.0.1a";
    db_ip_t ip = db_parse_ip(s0, strlen(s0), &code);
    assert_int_equal(ip, 0);
    assert_int_equal(code, DB_PARSE_IP_INVALID);

    char s1[] = "1a.0.0.1";
    ip = db_parse_ip(s1, strlen(s1), &code);
    assert_int_equal(ip, 0);
    assert_int_equal(code, DB_PARSE_IP_INVALID);

    char s2[] = "a1.0.0.1";
    ip = db_parse_ip(s2, strlen(s2), &code);
    assert_int_equal(ip, 0);
    assert_int_equal(code, DB_PARSE_IP_INVALID);

    char s3[] = "1.abc.0.1";
    ip = db_parse_ip(s3, strlen(s3), &code);
    assert_int_equal(ip, 0);
    assert_int_equal(code, DB_PARSE_IP_INVALID);
}

void test_db_parse_ip_depth(void **state) {
    int code = -1;

    char s0[] = "0";
    db_ip_t ip = db_parse_ip(s0, strlen(s0), &code);
    assert_int_equal(ip, 0);
    assert_int_equal(code, DB_PARSE_IP_INVALID);

    char s1[] = "0.0.0";
    ip = db_parse_ip(s1, strlen(s1), &code);
    assert_int_equal(ip, 0);
    assert_int_equal(code, DB_PARSE_IP_INVALID);

    char s2[] = "1.2.3.4.5";
    ip = db_parse_ip(s2, strlen(s2), &code);
    assert_int_equal(ip, 0);
    assert_int_equal(code, DB_PARSE_IP_INVALID);
}

void test_db_parse_next_record_correct_0(void **state) {
    char s[] = "\n\n1.2.3.4 www.baidu.com\n\n";
    char *p = s;
    dn_db_record_t res;
    int code = db_parse_next_record(&p, s + strlen(s), &res);
    assert_int_equal(code, DB_PARSE_RECORD_OKAY);

    char sn[] = "www.baidu.com";
    dn_db_name_t *name = db_parse_name(sn, strlen(sn), &code);
    for (dn_db_name_t *x = name, *y = res.name; x != 0 && y != 0; x = x->next, y = y->next) {
        assert_string_equal(x->label.label, y->label.label);
    }

    char si[] = "1.2.3.4";
    db_ip_t ip = db_parse_ip(si, strlen(si), &code);
    assert_int_equal(ip, res.ip);
}

void test_db_parse_next_record_correct_1(void **state) {
    char s[] = "\n\n\n192.168.3.1 @.localhost";
    char *p = s;
    dn_db_record_t res;
    int code = db_parse_next_record(&p, s + strlen(s), &res);
    assert_int_equal(code, DB_PARSE_RECORD_OKAY);

    char sn[] = "@.localhost";
    dn_db_name_t *name = db_parse_name(sn, strlen(sn), &code);
    for (dn_db_name_t *x = name, *y = res.name; x != 0 && y != 0; x = x->next, y = y->next) {
        assert_string_equal(x->label.label, y->label.label);
    }

    char si[] = "192.168.3.1";
    db_ip_t ip = db_parse_ip(si, strlen(si), &code);
    assert_int_equal(ip, res.ip);
}

void test_queue_empty(void **state) {
    queue_t *q = q_init(32);
    assert_true(q_empty(q));
    assert_false(q_full(q));
}

void test_queue_full(void **state) {
    queue_t *q = q_init(4);
    q_enq(q, 1);
    q_enq(q, 2);
    q_enq(q, 3);
    q_enq(q, 4);
    assert_true(q_full(q));
    q_deq(q);
    assert_false(q_full(q));
}

void test_queue_ok(void **state) {
    queue_t *q = q_init(4);
    q_enq(q, 1);
    q_enq(q, 2);
    q_enq(q, 3);
    q_enq(q, 4);
    assert_int_equal(q_deq(q), 1);
    assert_int_equal(q_deq(q), 2);
    assert_int_equal(q_deq(q), 3);
    assert_int_equal(q_deq(q), 4);
}

void test_queue_in_0(void **state) {
    queue_t *q = q_init(4);
    assert_false(q_in(q, 1));
    q_enq(q, 0);
    assert_false(q_in(q, 1));
    q_enq(q, 1);
    assert_true(q_in(q, 1));
    q_deq(q);
    assert_true(q_in(q, 1));
    q_deq(q);
    assert_false(q_in(q, 1));
}

void test_trie_init(void **state) {
    trie t = trie_init();
    assert_non_null(t);
}

void test_trie_works(void **state) {
    trie t = trie_init();
    char s1[] = "test the trie!";
    trie n1 = trie_insert(t, s1, s1);
    assert_ptr_equal(s1, trie_lookup(t, s1));
    trie n2 = trie_insert(t, s1, s1);
    assert_ptr_equal(s1, trie_lookup(t, s1));
    assert_ptr_equal(n1, n2);

    char s2[] = "test again";
    char s3[] = "test again and again";
    trie_insert(t, s2, s2);
    trie_insert(t, s3, s3);
    assert_ptr_equal(s2, trie_lookup(t, s2));
    assert_ptr_equal(s3, trie_lookup(t, s3));
}

int setup(void **state) {
    return 0;
}

int teardown(void **state) {
    return 0;
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_always_success),
        cmocka_unit_test(test_db_destroy_name_null_return_null),
        cmocka_unit_test(test_db_parse_name_correct_1),
        cmocka_unit_test(test_db_parse_name_correct_2),
        cmocka_unit_test(test_db_parse_name_null),
        cmocka_unit_test(test_db_parse_name_invalid),
        cmocka_unit_test(test_db_parse_ip_correct_0),
        cmocka_unit_test(test_db_parse_ip_correct_1),
        cmocka_unit_test(test_db_parse_ip_correct_2),
        cmocka_unit_test(test_db_parse_ip_correct_3),
        cmocka_unit_test(test_db_parse_ip_invalid_end),
        cmocka_unit_test(test_db_parse_ip_invalid_mid),
        cmocka_unit_test(test_db_parse_ip_invalid_non_digit),
        cmocka_unit_test(test_db_parse_ip_depth),
        cmocka_unit_test(test_db_parse_next_record_correct_0),
        cmocka_unit_test(test_db_parse_next_record_correct_1),
        cmocka_unit_test(test_queue_empty),
        cmocka_unit_test(test_queue_full),
        cmocka_unit_test(test_queue_ok),
        cmocka_unit_test(test_queue_in_0),
        cmocka_unit_test(test_trie_init),
        cmocka_unit_test(test_trie_works),
    };

    int count_fail_tests = cmocka_run_group_tests(tests, setup, teardown);

    return count_fail_tests;
}

