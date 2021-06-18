#include "test.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include <defines.h>
#include <kalos.h>
#include <kalos_dump.h>
#include <kalos_idl_compiler.h>
#include <kalos_lex.h>
#include <kalos_parse.h>
#include <kalos_run.h>
#include <kalos_string_format.h>
#include <kalos_string_system.h>
#include <kalos_value.h>

TEST_SUITE(kalos)

#define SCRIPT_DIR "test/data/scripts"
#define MAX_TOKENS 500

static int32_t total_allocated = 0;
static uint16_t allocation_id;
struct allocation_record {
    uint16_t id;
    size_t size;
};

static void* test_malloc(size_t size) {
    total_allocated += size;
    LOG("alloc #%d %d", allocation_id, size);
    // Stash the size in the allocation
    struct allocation_record info;
    info.size = size;
    info.id = allocation_id++;
    uint8_t* allocated = malloc(size + sizeof(info));
    memcpy(allocated, &info, sizeof(info));
    return allocated + sizeof(info);
}

static void test_free(void* memory) {
    uint8_t* allocated = memory;
    struct allocation_record info;
    allocated -= sizeof(info);
    memcpy(&info, allocated, sizeof(info));
    LOG("free #%d %d", info.id, info.size);
    total_allocated -= info.size;
    free(allocated);
}

static void* test_realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return test_malloc(size);
    }
    uint8_t* allocated = ptr;
    struct allocation_record info;
    allocated -= sizeof(info);
    memcpy(&info, allocated, sizeof(info));
    allocated = realloc(allocated, size + sizeof(info));
    total_allocated += (ssize_t)size - info.size;
    info.size = size;
    memcpy(allocated, &info, sizeof(info));
    return allocated + sizeof(info);
}

void read_buffer(char* file, char* buffer, int buffer_size) { 
    int fd = open(file, O_RDONLY);
    TEST_ASSERT_GREATER_THAN_INT_MESSAGE(-1, fd, file);
    int n = read(fd, buffer, buffer_size);
    close(fd);
    buffer[n] = 0;
}

void write_buffer(char* file, char* buffer) { 
    int fd = open(file, O_WRONLY|O_TRUNC);
    TEST_ASSERT_GREATER_THAN_INT_MESSAGE(-1, fd, file);
    int n = write(fd, buffer, strlen(buffer));
    close(fd);
}

kalos_module_parsed parse_modules_for_test() {
    static const char TEST_IDL[] = {
        #include "test_kalos.kidl.inc"
    };

    return kalos_idl_parse_module(TEST_IDL);
}

void lex_test(const char* file) {
    uint8_t script_buffer[1024];
    const int BUFFER_SIZE = 2048;

    char full_file[128] = {0};
    strcat(full_file, SCRIPT_DIR);
    strcat(full_file, "/");
    strcat(full_file, file);
    strcat(full_file, ".fdl");

    char* buffer = malloc(BUFFER_SIZE);
    read_buffer(full_file, buffer, BUFFER_SIZE);

    char outbuf[128];

    const char* s = buffer;
    if (s[0] == 'F') {
        // This is a failure test
        int expected_fail_line = strtol(++s, NULL, 10);
        while (*s++ != '\n') {}
        kalos_lex_state state = {0};
        kalos_lex_init(s, &state);

        for (int i = 0; i < MAX_TOKENS; i++) {
            int token = kalos_lex(&state, &outbuf[0]);
            if (token == KALOS_TOKEN_EOF) {
                TEST_FAIL_MESSAGE("Expected an error"); // Expected failure
            } else if (token == KALOS_TOKEN_ERROR) {
                TEST_ASSERT_EQUAL_MESSAGE(expected_fail_line, state.line, "Failure happened on the wrong line");
                goto success;
            }
        }
    } else {
        kalos_lex_state state = {0};
        kalos_lex_init(s, &state);

        for (int i = 0; i < MAX_TOKENS; i++) {
            int token = kalos_lex(&state, &outbuf[0]);
            // printf("%s (%s)\n", kalos_token_strings[token], outbuf);
            if (token == KALOS_TOKEN_EOF) {
                goto success;
            }
            TEST_ASSERT_NOT_EQUAL_MESSAGE(KALOS_TOKEN_ERROR, token, test_format_string("Got an error token on line %d! Remainder was '%.10s...'", state.line, state.s));
        }
        TEST_FAIL_MESSAGE("Parsed too many tokens");
    }

success:
    free(buffer);
}

void parse_test(const char* file) {
    uint8_t script_buffer[1024];
    const int BUFFER_SIZE = 4096;

    char full_file[128] = {0};
    strcat(full_file, SCRIPT_DIR);
    strcat(full_file, "/");
    strcat(full_file, file);
    strcat(full_file, ".fdl");

    char* buffer = malloc(BUFFER_SIZE);
    read_buffer(full_file, buffer, BUFFER_SIZE);

    char* s = buffer;
    if (s[0] == 'F') {
        // This is a failure test
        int expected_fail_line = strtol(++s, NULL, 10);
        while (*s++ != '\n') {}
        kalos_script script = {0};
        script.script_ops = &script_buffer[0];
        script.script_buffer_size = sizeof(script_buffer);

        kalos_parse_options options = {0};
        kalos_module_parsed parsed_modules = parse_modules_for_test();
        kalos_parse_result res = kalos_parse(buffer, parsed_modules, options, &script);
        kalos_idl_free_module(parsed_modules);

        TEST_ASSERT_FALSE_MESSAGE(res.success, "Expected an error");
        free(buffer);
        return;
    }

    kalos_script script = {0};
    script.script_ops = &script_buffer[0];
    script.script_buffer_size = sizeof(script_buffer);

    kalos_parse_options options = {0};
    kalos_module_parsed parsed_modules = parse_modules_for_test();
    kalos_parse_result res = kalos_parse(buffer, parsed_modules, options, &script);
    kalos_idl_free_module(parsed_modules);
    TEST_ASSERT_TRUE_MESSAGE(res.success, res.error);

    memset(buffer, 0, BUFFER_SIZE);
    kalos_dump(&script, buffer);

    char* buffer2 = malloc(BUFFER_SIZE);
    full_file[strlen(full_file) - 3] = 0;
    strcat(full_file, "bytecode");
    read_buffer(full_file, buffer2, BUFFER_SIZE);

    if (strcmp(buffer, buffer2) != 0) {
        printf("Expected:\n==================\n%s\nWas:\n==================\n%s\n", buffer2, buffer);
        // write_buffer(full_file, buffer);
        TEST_FAIL();
    }

    free(buffer);
    free(buffer2);
}

char output_buffer[1024];

void test_print(kalos_state state, int size, kalos_value* args) {
    while (size--) {
        if (args->type == KALOS_VALUE_STRING) {
            sprintf(output_buffer, "%s%s", output_buffer, kalos_string_c(state, args->value.string));
        } else if (args->type == KALOS_VALUE_NUMBER || args->type == KALOS_VALUE_BOOL) {
            sprintf(output_buffer, "%s%d", output_buffer, args->value.number);
        } else if (args->type == KALOS_VALUE_NONE) {
            sprintf(output_buffer, "%s(none)", output_buffer);
        } else if (args->type == KALOS_VALUE_OBJECT) {
            sprintf(output_buffer, "%s(object)", output_buffer);
        }
        args++;
    }
}

kalos_int test_kalos_sum_numbers(kalos_state state, int size, kalos_value* args) {
    int n = 0;
    for (int i = 0; i < size; i++) {
        n += args[i].value.number;
    }
    return n;
}

kalos_int test_kalos_sum_numbers_2(kalos_state state, kalos_int a, kalos_int b, int size, kalos_value* args) {
    int n = a+b;
    for (int i = 0; i < size; i++) {
        n += args[i].value.number;
    }
    return n;
}

kalos_string test_kalos_concat(kalos_state state, kalos_string* s1, kalos_string* s2, kalos_string* s3, kalos_string* s4, 
    kalos_string* s5, kalos_string* s6, kalos_string* s7, kalos_string* s8) {
    return kalos_string_allocate_fmt(state, "%s%s%s%s%s%s%s%s", 
        kalos_string_c(state, *s1), 
        kalos_string_c(state, *s2), 
        kalos_string_c(state, *s3), 
        kalos_string_c(state, *s4), 
        kalos_string_c(state, *s5), 
        kalos_string_c(state, *s6), 
        kalos_string_c(state, *s7), 
        kalos_string_c(state, *s8));
}

kalos_int test_read_only_read(kalos_state state) {
    return 42;
}

static kalos_int read_write_prop = 0;

kalos_int test_read_write_read(kalos_state state) {
    return read_write_prop;
}

void test_read_write_write(kalos_state state, kalos_int value) {
    read_write_prop = value;
}

void test_write_only_write(kalos_state state, kalos_int value) {
    sprintf(output_buffer, "%s%d", output_buffer, value);
}

kalos_string test_read_a(kalos_state state, kalos_object_ref* object) {
    return kalos_string_allocate_fmt(state, "a:%s", (*object)->context);
}

kalos_string test_read_b(kalos_state state, kalos_object_ref* object) {
    return kalos_string_allocate_fmt(state, "b:%s", (*object)->context);
}

static kalos_int c = 0;

kalos_int test_read_c(kalos_state state, kalos_object_ref* object) {
    return c;
}

kalos_int test_read_c2(kalos_state state, kalos_object_ref* object) {
    return ((const char*)(*object)->context)[0];
}

void test_write_c(kalos_state state, kalos_object_ref* object, kalos_int value) {
    c = value;
}

bool kalos_module_dispatch_test_test_object_a_props(kalos_state state, kalos_object_ref* object, int function, int param_count, kalos_stack* stack);
bool kalos_module_dispatch_test_test_object_b_props(kalos_state state, kalos_object_ref* object, int function, int param_count, kalos_stack* stack);

kalos_object_ref test_make_a(kalos_state state) {
    return kalos_allocate_prop_object(state, "a", kalos_module_dispatch_test_test_object_a_props);
}

kalos_object_ref test_make_b(kalos_state state) {
    return kalos_allocate_prop_object(state, "b", kalos_module_dispatch_test_test_object_b_props);
}

#include "test_kalos.dispatch.inc"

void run_test(const char* file) {
    if (strcmp(file, "test_varargs") == 0) {
        int x = 1;
    }
    uint8_t script_buffer[1024];
    const int BUFFER_SIZE = 2048;
    output_buffer[0] = 0;

    char full_file[128] = {0};
    strcat(full_file, SCRIPT_DIR);
    strcat(full_file, "/");
    strcat(full_file, file);
    strcat(full_file, ".fdl");

    char* buffer = malloc(BUFFER_SIZE);
    read_buffer(full_file, buffer, BUFFER_SIZE);

    kalos_script script = {0};
    script.script_ops = &script_buffer[0];
    script.script_buffer_size = sizeof(script_buffer);

    kalos_parse_options options = {0};
    kalos_module_parsed parsed_modules = parse_modules_for_test();
    kalos_parse_result res = kalos_parse(buffer, parsed_modules, options, &script);
    kalos_idl_free_module(parsed_modules);
    TEST_ASSERT_TRUE_MESSAGE(res.success, res.error);

    total_allocated = 0;
    kalos_fn fns = {0};
    fns.alloc = test_malloc;
    fns.realloc = test_realloc;
    fns.free = test_free;

    kalos_dispatch dispatch = {0};
    dispatch.modules = kalos_module_dispatch_test_dispatch;
    kalos_state state = kalos_init(&script, &dispatch, &fns);
    kalos_module_dispatch_test_trigger_init(state);
    kalos_string s = kalos_string_allocate(state, "hello world");
    kalos_module_dispatch_test_test_trigger_with_args(state, &s);
    kalos_run_free(state);
    TEST_ASSERT_EQUAL(0, total_allocated);

    char* buffer2 = malloc(BUFFER_SIZE);
    full_file[strlen(full_file) - 3] = 0;
    strcat(full_file, "output");
    read_buffer(full_file, buffer2, BUFFER_SIZE);

    if (strcmp(output_buffer, buffer2) != 0) {
        printf("Expected:\n==================\n%s\nWas:\n==================\n%s\n", buffer2, output_buffer);
        // write_buffer(full_file, output_buffer);
        TEST_FAIL();
    }
    free(buffer);
    free(buffer2);
    total_allocated = 0;
}

#define SCRIPT_TEST(x) \
    TEST(kalos_lex_##x) { lex_test("test_"#x); } \
    TEST(kalos_parse_##x) { parse_test("test_"#x); } \
    TEST(kalos_run_##x) { run_test("test_"#x); }

SCRIPT_TEST(automatic_conversions)
SCRIPT_TEST(barebones)
SCRIPT_TEST(builtins)
SCRIPT_TEST(comparisons)
SCRIPT_TEST(complex_expressions)
SCRIPT_TEST(const)
SCRIPT_TEST(global_local)
SCRIPT_TEST(handle_with_args)
SCRIPT_TEST(if)
SCRIPT_TEST(if_else)
SCRIPT_TEST(ignored_return)
SCRIPT_TEST(iterable)
SCRIPT_TEST(iterable_split)
SCRIPT_TEST(list)
SCRIPT_TEST(list_index)
SCRIPT_TEST(loop)
SCRIPT_TEST(module_const)
SCRIPT_TEST(module_prop)
SCRIPT_TEST(newtons_algo)
SCRIPT_TEST(object_props)
SCRIPT_TEST(object_props_write)
SCRIPT_TEST(operator_assignment)
SCRIPT_TEST(operator_precedence)
SCRIPT_TEST(self_trigger)
SCRIPT_TEST(store_multiple)
SCRIPT_TEST(store_object)
SCRIPT_TEST(string_formatting)
SCRIPT_TEST(string_literals)
SCRIPT_TEST(string_memory)
SCRIPT_TEST(string_replace)
SCRIPT_TEST(unary_expressions)
SCRIPT_TEST(varargs)

#define LEX_TEST_FAIL(x) \
    TEST(kalos_lex_fail_##x) { lex_test("fail_lex_" #x); }

LEX_TEST_FAIL(bad_escape)
LEX_TEST_FAIL(string_eof)
LEX_TEST_FAIL(string_eol)
LEX_TEST_FAIL(unknown_token)

#define PARSE_TEST_FAIL(x) \
    TEST(kalos_parse_fail_##x) { parse_test("fail_parse_" #x); }

PARSE_TEST_FAIL(break_without_loop)
PARSE_TEST_FAIL(invalid_const_global)
PARSE_TEST_FAIL(loop_var_undefined)
PARSE_TEST_FAIL(string_format_bad)
PARSE_TEST_FAIL(string_format_eof)
PARSE_TEST_FAIL(string_format_eol)

// Exhaustive test using python to generate the output test cases
TEST(kalos_string_format_run_exhaustive_test) {
    FILE* test = fopen("test/data/format/exhaustive_format_test.txt", "r");
    char line[256];
    int linenum = 0;

    total_allocated = 0;
    kalos_fn fns = {0};
    fns.alloc = test_malloc;
    fns.realloc = test_realloc;
    fns.free = test_free;

    kalos_state state = kalos_init_for_test(&fns);
    while (fgets(line, sizeof(line), test)) {
        linenum++;
        int value = strtol(line, NULL, 10);
        char* format_str = strchr(line, '\t') + 1;
        char* output = strchr(format_str, '\t') + 1;
        *(output - 1) = 0;
        *strchr(output, '\n') = 0;

        kalos_string_format format;
        TEST_ASSERT_TRUE(kalos_parse_string_format(format_str, &format));
        kalos_string str = kalos_string_format_int(state, value, &format);
        const char* out = kalos_string_c(state, str);
        TEST_ASSERT_NOT_NULL(out);
        if (strcmp(out, output) != 0) {
            printf("Failed: #%d %d |%s| Expected: |%s| Was: |%s|\n", linenum, value, format_str, output, out);
            return;
        } else {
            // printf("OK: #%d %d |%s| Expected: |%s| Was: |%s|\n", linenum, value, format_str, output, out);
        }
        kalos_string_release(state, str);
    }
    fclose(test);
    kalos_run_free(state);
    TEST_ASSERT_EQUAL(0, total_allocated);
    total_allocated = 0;
}

/*
#define SIZEOF(x) (sizeof(x) / sizeof(x[0]))

TEST(kalos_string_format_generate_exhaustive_test) {
    kalos_int inputs[] = { 0, 123, -123 };
    int widths[] = { 0, 2, 10 };
    char signs[] = { KALOS_STRING_FORMAT_SIGN_NEGATIVE, KALOS_STRING_FORMAT_SIGN_BOTH, KALOS_STRING_FORMAT_SIGN_POSITIVE_SPACE };
    char aligns[] = { KALOS_STRING_FORMAT_ALIGN_LEFT, KALOS_STRING_FORMAT_ALIGN_RIGHT, KALOS_STRING_FORMAT_ALIGN_CENTER };
    char alt_fmts[] = { 0, '#' };
    char fills[] = { ' ', '.' };
    char types[] = { 'd', 'x', 'o' };

    for (int i = 0; i < SIZEOF(inputs); i++) {
        kalos_int input = inputs[i];
        kalos_string_format format = {0};
        for (int j = 0; j < SIZEOF(widths); j++) {
            format.min_width = widths[j];
            for (int k = 0; k < SIZEOF(signs); k++) {
                format.sign = signs[k];
                for (int l = 0; l < SIZEOF(aligns); l++) {
                    format.align = aligns[l];
                    for (int m = 0; m < SIZEOF(alt_fmts); m++) {
                        format.alt_fmt = alt_fmts[m];
                        for (int n = 0; n < SIZEOF(fills); n++) {
                            format.fill = fills[n];
                            for (int o = 0; o < SIZEOF(types); o++) {
                                format.type = types[o];
                                char python_format[128];
                                sprintf(python_format, "python3 -c \"print(f'%d\\t%c%c%c%s%d%c\\t{%d:%c%c%c%s%d%c}')\" >> /tmp/out.txt\n", input, 
                                    format.fill, format.align, format.sign, format.alt_fmt ? "#" : "", format.min_width, format.type,
                                    input, format.fill, format.align, format.sign, format.alt_fmt ? "#" : "", format.min_width, format.type);
                                system(python_format);

                            }
                        }
                    }
                }
            }
        }
    }
}
*/
