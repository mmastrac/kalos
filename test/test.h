#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#include "unity/unity.h"

typedef void (*setup_fn)();
typedef void (*teardown_fn)();
typedef void (*test_fn)();

typedef struct test_data {
    test_fn fn;
    const char* name;
    int line;
} test_data;

typedef struct test_suite {
    const char* name;
    const char* file;
    setup_fn setup;
    teardown_fn teardown;
    test_data tests[256];
    int test_count; 
} test_suite;

void register_suite(test_suite* current_suite);

#define TEST_SUITE(suite) \
    test_suite suite_##suite = {#suite, __FILE__}; \
    static test_suite* current_suite = &suite_##suite; \
    __attribute__((constructor)) static void init_suite_##suite() { register_suite(current_suite); }

#define TEST(fn) \
    void test_##fn();\
    __attribute__((constructor)) static void init_##fn() { register_test(current_suite, test_##fn, #fn, __LINE__); } void test_##fn()

static void register_test(test_suite* current_suite, void(*fn)(), const char* name, int line) {
    // printf("%s:\n", name);
    test_data test = { fn, name, line };
    current_suite->tests[current_suite->test_count++] = test;
    // int retval = fn();
    // if (retval < 0) {
    //     printf("  failed!\n");
    // } else {
    //     printf("  PASS\n");
    // }
}

static char* test_format_string(const char* fmt, ...) {
    static char temp[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf(temp, fmt, args);
    va_end(args);

    return temp;
}
