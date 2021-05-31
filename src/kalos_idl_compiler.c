#include <stdbool.h>

#include "kalos_module.h"
#include "kalos_idl_parse.h"

static const char* kalos_idl_script = "\
var module = "";\
\
handle module {\
    \
}\
"; 

struct kalos_module_builder {
    kalos_export* exports;
    int export_count;
    int function_arg_count;
};

static void new_export(struct kalos_module_builder* builder) {
    builder->exports = realloc(builder->exports, (++builder->export_count) * sizeof(kalos_export));
    memset((void*)&builder->exports[builder->export_count - 1], 0, sizeof(kalos_export));
}

static kalos_export* current_export(struct kalos_module_builder* builder) {
    return &builder->exports[builder->export_count - 1];
}

static kalos_function_type strtotype(const char* s) {
    if (strcmp(s, "number") == 0) {
        return FUNCTION_TYPE_NUMBER;
    }
    if (strcmp(s, "string") == 0) {
        return FUNCTION_TYPE_STRING;
    }
    if (strcmp(s, "bool") == 0) {
        return FUNCTION_TYPE_BOOL;
    }
    if (strcmp(s, "object") == 0) {
        return FUNCTION_TYPE_OBJECT;
    }
    if (strcmp(s, "any") == 0) {
        return FUNCTION_TYPE_ANY;
    }
    if (strcmp(s, "varargs") == 0) {
        return FUNCTION_TYPE_VARARGS;
    }

    return FUNCTION_TYPE_VOID;
}

static void begin_module(void* context, const char* module) {
    LOG("module %s", module);
}

static void end_module(void* context, const char* module) {
    LOG("%s", module);
}

static void begin_function(void* context, const char* name) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->type = KALOS_EXPORT_TYPE_FUNCTION;
    current_export(builder)->name = strdup(name);
    LOG("fn %s", name);
}

static void function_arg(void* context, const char* name, const char* type) {
    struct kalos_module_builder* builder = context;
    int arg = current_export(builder)->entry.function.arg_count++;
    current_export(builder)->entry.function.args[arg].name = strdup(name);
    current_export(builder)->entry.function.args[arg].type = strtotype(type);
    LOG("arg %s %s", name, type);
}

static void end_function(void* context, const char* name, const char* type, const char* symbol) {
    struct kalos_module_builder* builder = context;
    current_export(builder)->entry.function.return_type = strtotype(type);
    current_export(builder)->entry.function.symbol = strdup(symbol);
    LOG("fn %s %s %s", name, type, symbol);
}

static void constant_symbol(void* context, const char* name, const char* type, const char* symbol) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name = strdup(name);
    current_export(builder)->entry.const_symbol = strdup(symbol); 
    if (strcmp(type, "number") == 0) {
        current_export(builder)->type = KALOS_EXPORT_TYPE_CONST_NUMBER;
    } else if (strcmp(type, "string") == 0) {
        current_export(builder)->type = KALOS_EXPORT_TYPE_CONST_STRING;
    }
    LOG("const %s %s %s", name, type, symbol);
}

static void constant_string(void* context, const char* name, const char* type, const char* string) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name = strdup(name);
    current_export(builder)->entry.const_string = strdup(string);
    LOG("const %s %s %s", name, type, string);
}

static void constant_number(void* context, const char* name, const char* type, kalos_int number) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name = strdup(name);
    current_export(builder)->type = KALOS_EXPORT_TYPE_CONST_NUMBER;
    current_export(builder)->entry.const_number = number;
    LOG("const %s %s %d", name, type, number);
}

static void property(void* context, const char* name, const char* type, const char* mode, const char* symbol) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name = strdup(name);
    current_export(builder)->entry.function.symbol = strdup(symbol);
        current_export(builder)->entry.function.return_type = strtotype(type);
    if (strcmp(mode, "read") == 0) {
        current_export(builder)->type = KALOS_EXPORT_TYPE_PROP_READ;
    } else if (strcmp(mode, "write") == 0) {
        current_export(builder)->type = KALOS_EXPORT_TYPE_PROP_WRITE;
    }
    LOG("prop %s %s %s %s", name, type, mode, symbol);
}

int export_compare(const void* v1, const void* v2) {
    const kalos_export* e1 = v1;
    const kalos_export* e2 = v2;
    int cmp = strcmp(e1->name, e2->name);
    if (cmp != 0) {
        return cmp;
    }
    return e1->type - e2->type;
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
        printf("fail!");
    }
    qsort(context.exports, context.export_count, sizeof(context.exports[0]), export_compare);
    for (int i = 0; i < context.export_count; i++) {
        printf("%s\n", context.exports[i].name);
    }
    kalos_module module = {0};
    return module;
}
