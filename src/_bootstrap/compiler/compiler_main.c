#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include "../kalos_parse.h"
#include "compiler_gen.h"
#include "compiler_idl.h"

static int verbose = 0;
static FILE* output_file;

static int32_t total_allocated = 0;
static uint16_t allocation_id;
struct allocation_record {
    uint16_t id;
    size_t size;
};

static void* internal_malloc(size_t size) {
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

static void internal_free(void* memory) {
    uint8_t* allocated = memory;
    struct allocation_record info;
    allocated -= sizeof(info);
    memcpy(&info, allocated, sizeof(info));
    // LOG("free #%d %d", info.id, info.size);
    total_allocated -= info.size;
    free(allocated);
}

static void* internal_realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return internal_malloc(size);
    }
    uint8_t* allocated = ptr;
    struct allocation_record info;
    allocated -= sizeof(info);
    memcpy(&info, allocated, sizeof(info));
    allocated = realloc(allocated, size + sizeof(info));
    total_allocated += (ssize_t)size - (ssize_t)info.size;
    info.size = size;
    memcpy(allocated, &info, sizeof(info));
    return allocated + sizeof(info);
}

static void internal_error(void* context, int line, const char* error) {
    if (line) {
        printf("ERROR on line %d: %s\n", line, error);
    } else {
        printf("ERROR: %s\n", error);
    }
    exit(1);
}

static void internal_print(void* context, const char* s) {
    fprintf(output_file, "%s", s);
}

kalos_state compiler_env = {
    .alloc = internal_malloc,
    .realloc = internal_realloc,
    .free = internal_free,
    .print = internal_print,
    .error = internal_error,
};

int run_script(const char* input_data, kalos_int argc, const char* argv[]) {
    output_file = stdout;
    kalos_module_parsed modules = kalos_idl_parse_module(compiler_kidl_text_inc(), &compiler_env);
    kalos_buffer script;
    kalos_parse_options options = {0};
    kalos_parse_result res = kalos_parse_buffer(input_data, modules, options, &compiler_env, &script);
    if (!res.success) {
        printf("ERROR on line %d: %s\n", res.line, res.error);
        exit(1);
    }
    kalos_dispatch dispatch = {0};
    dispatch.dispatch_name = kalos_module_idl_dynamic_dispatch;
    kalos_run_state* state = kalos_init(script.buffer, &dispatch, &compiler_env);
    kalos_object_ref args = kalos_allocate_string_iterable(kalos_state_from_run_state(state), argv, argc);
    kalos_module_idl_sys_trigger_main(state, &args);
    kalos_run_free(state);
    kalos_buffer_free(script);
    kalos_buffer_free(modules);
    return 0;
}

void log_printf(const char* fmt, ...) {
    if (verbose) {
        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
        fputc('\n', stdout);
    }
}

#ifndef IS_TEST
int main(int argc, const char** argv) {
    run_script(compiler_script_text_inc(), argc, argv);

    return 0;
}
#endif
