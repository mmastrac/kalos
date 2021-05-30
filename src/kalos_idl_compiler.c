#include <stdbool.h>

#include "kalos_module.h"
#include "kalos_idl_parse.h"

struct kalos_module_builder {
    kalos_export current;
};

static void begin_module(void* context, const char* module) {
    printf("module %s\n", module);
}

static void end_module(void* context, const char* module) {
    printf("%s\n", module);
}

static void begin_function(void* context, const char* name) {
    printf("fn %s\n", name);
}

static void function_arg(void* context, const char* name, const char* type) {
    printf("arg %s %s\n", name, type);
}

static void end_function(void* context, const char* name, const char* type, const char* symbol) {
    printf("fn %s %s %s\n", name, type, symbol);
}

static void constant_symbol(void* context, const char* name, const char* type, const char* symbol) {
    printf("const %s %s %s\n", name, type, symbol);
}

static void constant_string(void* context, const char* name, const char* type, const char* string) {
    printf("const %s %s %s\n", name, type, string);
}

static void constant_number(void* context, const char* name, const char* type, kalos_int number) {
    printf("const %s %s %d\n", name, type, number);
}

static void property(void* context, const char* name, const char* type, const char* mode, const char* symbol) {
    printf("prop %s %s %s %s\n", name, type, mode, symbol);

}

kalos_module kalos_idl_parse_module(const char* s) {
    kalos_idl_callbacks callbacks = {
        begin_module,
        end_module,
        begin_function,
        function_arg,
        end_function,
        constant_symbol,
        constant_string,
        constant_number,
        property,
    };

    struct kalos_module_builder context = {0};

    if (!kalos_idl_parse_callback(s, &context, &callbacks)) {
        printf("fail!\n");
    }
    kalos_module module = {0};
    return module;
}
