#include "test.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include <kalos.h>
#include <kalos_parse.h>
#include <kalos_run.h>
#include <_kalos_defines.h>
#include <_kalos_lex.h>
#include <_kalos_script.h>
#include <_kalos_string_format.h>
#include <_kalos_string_system.h>
#include <_kalos_value.h>
#include <compiler/compiler_idl.h>
#include <modules/kalos_module_file.h>

TEST_SUITE(kalos)

#define SCRIPT_DIR "test/data/scripts"

static int32_t total_allocated = 0;
static uint16_t allocation_id;
struct allocation_record {
    uint16_t id;
    size_t size;
};

static void* test_malloc(size_t size) {
    total_allocated += size;
    // LOG("alloc #%d %d", allocation_id, size);
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
    // LOG("free #%d %d", info.id, info.size);
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
    if (size > info.size) {
        total_allocated += size - info.size;
    } else {
        total_allocated -= info.size - size;
    }
    info.size = size;
    memcpy(allocated, &info, sizeof(info));
    return allocated + sizeof(info);
}

static void test_error(void* context, int line, const char* error) {
    LOG("Error reported: %d %s", line, error);
}

static void test_print_string(void* context, const char* s) {
    LOG("print: %s", s);
}

kalos_state test_env = {
    .alloc = test_malloc,
    .realloc = test_realloc,
    .free = test_free,
    .error = test_error,
    .print = test_print_string,
};

kalos_buffer read_buffer(const char* input) {
    FILE* fd = fopen(input, "rb");
    ASSERT(fd);
    fseek(fd, 0, SEEK_END);
    off_t size = ftell(fd);
    kalos_buffer buffer = kalos_buffer_alloc(&test_env, size + 1);
    fseek(fd, 0, SEEK_SET);
    fread((void*)buffer.buffer, size, 1, fd);
    fclose(fd);
    buffer.buffer[size] = 0;
    return buffer;
}

void write_buffer(const char* file, char* buffer) { 
    int fd = open(file, O_WRONLY|O_TRUNC);
    TEST_ASSERT_GREATER_THAN_INT_MESSAGE(-1, fd, file);
    int n = write(fd, buffer, strlen(buffer));
    close(fd);
}

const char* make_test_filename(const char* name, const char* extension) {
    static char filename[128] = {0};
    memset(filename, 0, sizeof(filename));
    TEST_ASSERT_LESS_THAN(sizeof(filename), snprintf(filename, sizeof(filename), "%s/%s.%s", SCRIPT_DIR, name, extension));
    return filename;
}

kalos_module_parsed parse_modules_for_test() {
    static uint8_t parsed_modules[10 * 1024] = {0};
    static size_t parsed_modules_size = 0;

    if (!parsed_modules_size) {
        kalos_buffer buffer = read_buffer("test/test_kalos.kidl");
        kalos_module_parsed parsed = kalos_idl_parse_module((const char*)buffer.buffer, &test_env);
        ASSERT(parsed.buffer);
        kalos_buffer_free(buffer);
        memcpy(parsed_modules, parsed.buffer, kalos_buffer_size(parsed));
        parsed_modules_size = kalos_buffer_size(parsed);
        kalos_buffer_free(parsed);
    }

    kalos_module_parsed parsed = kalos_buffer_alloc(&test_env, parsed_modules_size);
    memcpy(parsed.buffer, &parsed_modules[0], parsed_modules_size);
    return parsed;
}

kalos_loaded_script test_script_loader(const char* base_name, kalos_load_result* result) {
    kalos_buffer buffer = read_buffer(make_test_filename(base_name, "fdl"));
    kalos_loaded_script script = {0};
    script.context = (void*)buffer.buffer;
    script.text = (const char*)buffer.buffer;
    *result = SCRIPT_LOAD_SUCCESS; // TODO: test failure
    return script;
}

void test_script_unloader(kalos_loaded_script script) {
    kalos_buffer buffer = {.buffer = script.context};
    kalos_buffer_free(buffer);
}

kalos_parse_result parse_test_runner(kalos_buffer script_text, const char* bytecode_file, kalos_buffer bytecode) {
    kalos_buffer script;
    kalos_parse_options options = {.loader = test_script_loader, .unloader = test_script_unloader};
    kalos_module_parsed parsed_modules = parse_modules_for_test();
    kalos_parse_result res = kalos_parse_buffer((const char*)script_text.buffer, parsed_modules, options, &test_env, &script);
    kalos_buffer_free(parsed_modules);
    if (res.error) {
        // Don't check bytecode if we failed to parse
        kalos_buffer_free(script);
        return res;
    } else {
        TEST_ASSERT_NOT_NULL_MESSAGE(bytecode.buffer, "Expected this test to fail but it didn't");
    }

    kalos_buffer dump_buffer = kalos_dump_to_buffer(&test_env, (kalos_script)script.buffer);

    if (strcmp((const char*)dump_buffer.buffer, (const char*)bytecode.buffer) != 0) {
        printf("Expected:\n==================\n%s\nWas:\n==================\n%s\n", bytecode.buffer, dump_buffer.buffer);
        // write_buffer(bytecode_file, dump_buffer);
        TEST_FAIL();
    }

    kalos_buffer_free(script);
    kalos_buffer_free(dump_buffer);
    return res;
}

void parse_test(const char* file) {
    total_allocated = 0;
    kalos_buffer bytecode = {NULL};
    kalos_buffer script = read_buffer(make_test_filename(file, "fdl"));
    bool is_failure_test = strncmp((const char*)script.buffer, "# FAIL ", 7) == 0;
    if (!is_failure_test) {
        bytecode = read_buffer(make_test_filename(file, "bytecode"));
    }
    kalos_parse_result result = parse_test_runner(script, make_test_filename(file, "bytecode"), bytecode);
    if (is_failure_test) {
        // This is a failure case!
        char error_buf[128] = {0};
        TEST_ASSERT_LESS_THAN(sizeof(error_buf), snprintf(error_buf, sizeof(error_buf), "# FAIL %d %s", result.line, result.error));
        *strchr((const char*)script.buffer, '\n') = 0;
        TEST_ASSERT_TRUE_MESSAGE(result.error, "Expected a failure");
        TEST_ASSERT_EQUAL_STRING(error_buf, script.buffer);
    } else {
        // Expected success
        if (!result.success) {
            char error_buf[128] = {0};
            TEST_ASSERT_LESS_THAN(sizeof(error_buf), snprintf(error_buf, sizeof(error_buf), "Error on line %d: %s", result.line, result.error));
            TEST_ASSERT_TRUE_MESSAGE(result.success, error_buf);
        }
    }
    kalos_buffer_free(script);
    kalos_buffer_free(bytecode);
    TEST_ASSERT_EQUAL(0, total_allocated);
}

char output_buffer[10*1024];

void test_print(kalos_state* state, int size, kalos_value* args) {
    ASSERT(strlen(output_buffer) < sizeof(output_buffer) - 1024);
    while (size--) {
        if (args->type == KALOS_VALUE_STRING) {
            sprintf(output_buffer, "%s%s", output_buffer, kalos_string_c(state, args->value.string));
        } else if (args->type == KALOS_VALUE_NUMBER || args->type == KALOS_VALUE_BOOL) {
            sprintf(output_buffer, "%s%d", output_buffer, args->value.number);
        } else if (args->type == KALOS_VALUE_NONE) {
            sprintf(output_buffer, "%s(none)", output_buffer);
        } else if (args->type == KALOS_VALUE_OBJECT) {
            if (kalos_coerce(state, args, KALOS_VALUE_STRING)) {
                sprintf(output_buffer, "%s%s", output_buffer, kalos_string_c(state, args->value.string));
            } else {
                sprintf(output_buffer, "%s(object)", output_buffer);
            }
        }
        args++;
    }
}

kalos_int test_kalos_sum_numbers(kalos_state* state, int size, kalos_value* args) {
    int n = 0;
    for (int i = 0; i < size; i++) {
        n += args[i].value.number;
    }
    return n;
}

kalos_int test_kalos_sum_numbers_2(kalos_state* state, kalos_int a, kalos_int b, int size, kalos_value* args) {
    int n = a+b;
    for (int i = 0; i < size; i++) {
        n += args[i].value.number;
    }
    return n;
}

kalos_string test_kalos_concat(kalos_state* state, kalos_string* s1, kalos_string* s2, kalos_string* s3, kalos_string* s4, 
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

kalos_int test_read_only_read(kalos_state* state) {
    return 42;
}

static kalos_int read_write_prop = 0;

kalos_int test_read_write_read(kalos_state* state) {
    return read_write_prop;
}

void test_read_write_write(kalos_state* state, kalos_int value) {
    read_write_prop = value;
}

void test_write_only_write(kalos_state* state, kalos_int value) {
    sprintf(output_buffer, "%s%d", output_buffer, value);
}

kalos_string test_read_a(kalos_state* state, kalos_object_ref* object) {
    return kalos_string_allocate_fmt(state, "a:%s", (*object)->context);
}

kalos_string test_read_b(kalos_state* state, kalos_object_ref* object) {
    return kalos_string_allocate_fmt(state, "b:%s", (*object)->context);
}

static kalos_int c = 0;

kalos_int test_read_c(kalos_state* state, kalos_object_ref* object) {
    return c;
}

kalos_int test_read_c2(kalos_state* state, kalos_object_ref* object) {
    return ((const char*)(*object)->context)[0];
}

void test_write_c(kalos_state* state, kalos_object_ref* object, kalos_int value) {
    c = value;
}

kalos_object_dispatch kalos_module_dispatch_test_test_object_a_props;
kalos_object_dispatch kalos_module_dispatch_test_test_object_b_props;

kalos_object_ref test_make_a(kalos_state* state) {
    return kalos_allocate_prop_object(state, "a", &kalos_module_dispatch_test_test_object_a_props);
}

kalos_object_ref test_make_b(kalos_state* state) {
    return kalos_allocate_prop_object(state, "b", &kalos_module_dispatch_test_test_object_b_props);
}

#include "test_kalos.dispatch.inc"

void run_test(const char* name) {
    total_allocated = 0;
    const int BUFFER_SIZE = 2048;
    output_buffer[0] = 0;

    kalos_buffer buffer = read_buffer(make_test_filename(name, "fdl"));
    kalos_parse_options options = {.loader = test_script_loader, .unloader = test_script_unloader};
    kalos_module_parsed parsed_modules = parse_modules_for_test();
    kalos_buffer script = kalos_buffer_alloc(&test_env, BUFFER_SIZE);
    kalos_parse_result res = kalos_parse((const char*)buffer.buffer, parsed_modules, options, (kalos_script)script.buffer, BUFFER_SIZE);
    kalos_buffer_free(parsed_modules);
    TEST_ASSERT_TRUE_MESSAGE(res.success, res.error);

    kalos_dispatch dispatch = {0};
    dispatch.modules = kalos_module_dispatch_test_dispatch;
    kalos_run_state* state = kalos_init((const_kalos_script)script.buffer, &dispatch, &test_env);
    kalos_module_dispatch_test_trigger_init(state);
    kalos_string s = kalos_string_allocate((kalos_state*)state, "hello world");
    kalos_module_dispatch_test_test_trigger_with_args(state, &s);
    kalos_run_free(state);

    kalos_buffer output = read_buffer(make_test_filename(name, "output"));

    if (strcmp(output_buffer, (const char*)output.buffer) != 0) {
        printf("Expected:\n==================\n%s\nWas:\n==================\n%s\n", output.buffer, output_buffer);
        // write_buffer(full_file, output_buffer);
        TEST_FAIL();
    }
    kalos_buffer_free(output);
    kalos_buffer_free(buffer);
    kalos_buffer_free(script);
    TEST_ASSERT_EQUAL(0, total_allocated);
}

#define SCRIPT_TEST(x) \
    TEST(kalos_parse_##x) { parse_test("test_"#x); } \
    TEST(kalos_run_##x) { run_test("test_"#x); }

SCRIPT_TEST(automatic_conversions)
SCRIPT_TEST(barebones)
SCRIPT_TEST(builtins)
SCRIPT_TEST(comparisons)
SCRIPT_TEST(complex_expressions)
SCRIPT_TEST(const)
SCRIPT_TEST(file)
SCRIPT_TEST(fn)
SCRIPT_TEST(fn_ret)
SCRIPT_TEST(global_local)
SCRIPT_TEST(handler_with_args)
SCRIPT_TEST(head_tail)
SCRIPT_TEST(idl)
SCRIPT_TEST(if)
SCRIPT_TEST(if_else)
SCRIPT_TEST(ignored_return)
SCRIPT_TEST(import)
SCRIPT_TEST(iterable)
SCRIPT_TEST(iterable_split)
SCRIPT_TEST(list)
SCRIPT_TEST(list_index)
SCRIPT_TEST(list_nested)
SCRIPT_TEST(loop)
SCRIPT_TEST(module_const)
SCRIPT_TEST(module_prop)
SCRIPT_TEST(newtons_algo)
SCRIPT_TEST(object_props)
SCRIPT_TEST(object_props_write)
SCRIPT_TEST(operator_assignment)
SCRIPT_TEST(operator_precedence)
SCRIPT_TEST(overload)
SCRIPT_TEST(self_trigger)
SCRIPT_TEST(store_multiple)
SCRIPT_TEST(store_object)
SCRIPT_TEST(string_formatting)
SCRIPT_TEST(string_index)
SCRIPT_TEST(string_literals)
SCRIPT_TEST(string_memory)
SCRIPT_TEST(string_replace)
SCRIPT_TEST(unary_expressions)
SCRIPT_TEST(varargs)

#define PARSE_TEST_FAIL(x) \
    TEST(kalos_parse_fail_##x) { parse_test("fail_parse_" #x); }

PARSE_TEST_FAIL(bad_escape)
PARSE_TEST_FAIL(bad_return)
PARSE_TEST_FAIL(bad_return2)
PARSE_TEST_FAIL(bad_return3)
PARSE_TEST_FAIL(break_without_loop)
PARSE_TEST_FAIL(invalid_const_global)
PARSE_TEST_FAIL(loop_var_undefined)
PARSE_TEST_FAIL(string_eof)
PARSE_TEST_FAIL(string_eol)
PARSE_TEST_FAIL(string_format_bad)
PARSE_TEST_FAIL(string_format_eof)
PARSE_TEST_FAIL(string_format_eol)
PARSE_TEST_FAIL(unknown_token)

TEST(parse_modules_for_test) {
    total_allocated = 0;
    kalos_module_parsed modules = parse_modules_for_test();
    kalos_buffer_free(modules);
    TEST_ASSERT_EQUAL(0, total_allocated);
}

TEST(kalos_idl_parser_smoke_test) {
    total_allocated = 0;
    kalos_buffer buffer = read_buffer("test/data/idl/smoketest.kidl");
    kalos_module_parsed modules = kalos_idl_parse_module((const char*)buffer.buffer, &test_env);
    kalos_buffer_free(buffer);
    kalos_buffer_free(modules);
    TEST_ASSERT_EQUAL(0, total_allocated);
}

TEST(kalos_idl_parser_many_consts) {
    total_allocated = 0;
    kalos_buffer buffer = read_buffer("test/data/idl/largeconsts.kidl");
    kalos_module_parsed modules = kalos_idl_parse_module((const char*)buffer.buffer, &test_env);
    kalos_buffer_free(buffer);
    kalos_buffer_free(modules);
    TEST_ASSERT_EQUAL(0, total_allocated);
}

// Exhaustive test using python to generate the output test cases
TEST(kalos_string_format_run_exhaustive_test) {
    total_allocated = 0;
    FILE* test = fopen("test/data/format/exhaustive_format_test.txt", "r");
    char line[256];
    int linenum = 0;

    kalos_state* state = &test_env;
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
    TEST_ASSERT_EQUAL(0, total_allocated);
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
