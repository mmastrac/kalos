#include <stdbool.h>

#include "kalos_dump.h"
#include "kalos_module.h"
#include "kalos_parse.h"
#include "kalos_run.h"
#include "kalos_idl_compiler.h"
#include "kalos_idl_parse.h"

struct kalos_module_builder {
    void* kalos_module_buffer;
    size_t module_buffer_size;
    size_t module_count;
    kalos_int module_name_index;
    kalos_int prefix_index;
    char* string_buffer;
    size_t string_buffer_size;
    size_t string_buffer_index;
    kalos_export* exports;
    size_t export_count;
    size_t function_index;
    size_t function_arg_count;
};

int export_compare(void* context, const void* v1, const void* v2) {
    struct kalos_module_builder* builder = context;
    const kalos_export* e1 = v1;
    const kalos_export* e2 = v2;
    int cmp = strcmp(e1->name_index + builder->string_buffer, e2->name_index + builder->string_buffer);
    if (cmp != 0) {
        return cmp;
    }

    return (int)e1->type - (int)e2->type;
}

static void new_export(struct kalos_module_builder* builder) {
    builder->exports = realloc(builder->exports, (++builder->export_count) * sizeof(kalos_export));
    memset((void*)&builder->exports[builder->export_count - 1], 0, sizeof(kalos_export));
}

static kalos_export* current_export(struct kalos_module_builder* builder) {
    return &builder->exports[builder->export_count - 1];
}

static kalos_int strpack(struct kalos_module_builder* builder, const char* s) {
    size_t l = strlen(s) + 1;
    while (builder->string_buffer_size - builder->string_buffer_index < l) {
        if (builder->string_buffer_size == 0) {
            builder->string_buffer_size = 16;
        }
        builder->string_buffer_size *= 2;
        builder->string_buffer = realloc(builder->string_buffer, builder->string_buffer_size);
    }
    strcpy(builder->string_buffer + builder->string_buffer_index, s);
    builder->string_buffer_index += l;
    return (kalos_int)(builder->string_buffer_index - l); // offset into buffer
}

static char* unstring(char* s) {
    int len = strlen(s);
    int out = 0;
    for (int i = 1; i < len - 1; i++) {
        s[out] = s[i];
        out++;
    }
    s[out] = 0;
    return s;
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

static void prefix(void* context, const char* prefix) {
    struct kalos_module_builder* builder = context;
    builder->prefix_index = strpack(builder, unstring((char*)prefix));
}

static void begin_module(void* context, const char* module) {
    struct kalos_module_builder* builder = context;
    builder->export_count = 0;
    builder->module_count++;
    builder->module_name_index = strpack(builder, module);
    builder->function_index = 1;
    LOG("module %s", module);
}

static void end_module(void* context) {
    struct kalos_module_builder* builder = context;
    qsort_r(builder->exports, builder->export_count, sizeof(kalos_export), context, export_compare);
    size_t this_module_size = sizeof(kalos_module) + sizeof(kalos_export) * builder->export_count;
    builder->module_buffer_size += this_module_size;
    builder->kalos_module_buffer = realloc(builder->kalos_module_buffer, builder->module_buffer_size);

    kalos_module* module = (kalos_module*)((uint8_t*)builder->kalos_module_buffer + builder->module_buffer_size - this_module_size);
    module->index = builder->module_count - 1;
    module->name_index = builder->module_name_index;
    module->prefix_index = builder->prefix_index;
    module->export_count = builder->export_count;
    memcpy((uint8_t*)module + sizeof(kalos_module), builder->exports, builder->export_count * sizeof(kalos_export));
    free(builder->exports);
    builder->exports = 0;
    LOG("end");
}

static void begin_function(void* context, const char* name) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->index = builder->export_count - 1;
    current_export(builder)->type = KALOS_EXPORT_TYPE_FUNCTION;
    current_export(builder)->name_index = strpack(builder, name);
    current_export(builder)->entry.function.invoke_id = builder->function_index++;
    current_export(builder)->entry.function.vararg_type = FUNCTION_TYPE_VOID;
    LOG("fn %s", name);
}

static void function_arg(void* context, const char* name, const char* type, bool is_varargs) {
    struct kalos_module_builder* builder = context;
    if (is_varargs) {
        current_export(builder)->entry.function.vararg_type = strtotype(type);
    } else {
        int arg = current_export(builder)->entry.function.arg_count++;
        current_export(builder)->entry.function.args[arg].name_index = strpack(builder, name);
        current_export(builder)->entry.function.args[arg].type = strtotype(type);
    }
    LOG("arg %s %s", name, type);
}

static void end_function(void* context, const char* name, const char* type, const char* symbol) {
    struct kalos_module_builder* builder = context;
    current_export(builder)->entry.function.return_type = strtotype(type);
    current_export(builder)->entry.function.symbol_index = strpack(builder, symbol);
    LOG("fn %s %s %s", name, type, symbol);
}

static void constant_string(void* context, const char* name, const char* type, const char* s) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name_index = strpack(builder, name);
    current_export(builder)->type = KALOS_EXPORT_TYPE_CONST_STRING;
    current_export(builder)->entry.const_string_index = strpack(builder, unstring((char*)s));
    LOG("const %s %s %s", name, type, s);
}

static void constant_number(void* context, const char* name, const char* type, kalos_int number) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name_index = strpack(builder, name);
    current_export(builder)->type = KALOS_EXPORT_TYPE_CONST_NUMBER;
    current_export(builder)->entry.const_number = number;
    LOG("const %s %s %d", name, type, number);
}

static void property(void* context, const char* name, const char* type, const char* mode, const char* symbol, const char* symbol2) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name_index = strpack(builder, name);
    current_export(builder)->type = KALOS_EXPORT_TYPE_PROPERTY;
    current_export(builder)->entry.property.type = strtotype(type);
    if (strcmp(mode, "read") == 0) {
        current_export(builder)->entry.property.read_invoke_id = builder->function_index++;
        current_export(builder)->entry.property.read_symbol_index = strpack(builder, symbol);
    } else if (strcmp(mode, "write") == 0) {
        current_export(builder)->entry.property.write_invoke_id = builder->function_index++;
        current_export(builder)->entry.property.write_symbol_index = strpack(builder, symbol);
    } else if (strcmp(mode, "read,write") == 0) {
        current_export(builder)->entry.property.read_invoke_id = builder->function_index++;
        current_export(builder)->entry.property.read_symbol_index = strpack(builder, symbol);
        current_export(builder)->entry.property.write_invoke_id = builder->function_index++;
        current_export(builder)->entry.property.write_symbol_index = strpack(builder, symbol2);
    }
    LOG("prop %s %s %s %s %s", name, type, mode, symbol, symbol2);
}

kalos_module_parsed kalos_idl_parse_module(const char* s) {
    kalos_idl_callbacks callbacks = {
        prefix,
        begin_module,
        end_module,
        begin_function,
        function_arg,
        end_function,
        constant_string,
        constant_number,
        property
    };

    struct kalos_module_builder context = {0};

    if (!kalos_idl_parse_callback(s, &context, &callbacks)) {
        printf("fail!");
    }

    context.kalos_module_buffer = realloc(context.kalos_module_buffer, sizeof(kalos_module_header) + context.module_buffer_size + context.string_buffer_index);
    memmove((uint8_t*)context.kalos_module_buffer + sizeof(kalos_module_header), (uint8_t*)context.kalos_module_buffer, context.module_buffer_size);
    kalos_module_header* header = (kalos_module_header*)context.kalos_module_buffer;
    header->version = 1;
    header->module_count = context.module_count;
    header->module_size = context.module_buffer_size;
    header->module_offset = sizeof(kalos_module_header);
    header->string_offset = header->module_size + header->module_offset;
    header->string_size = context.string_buffer_index;
    memcpy((uint8_t*)context.kalos_module_buffer + header->string_offset, context.string_buffer, context.string_buffer_index);
    free(context.string_buffer);

    kalos_module_parsed parsed = { .data=context.kalos_module_buffer, .size=sizeof(kalos_module_header) + context.module_buffer_size + context.string_buffer_index };
    return parsed;
}

static const char IDL_COMPILER_SCRIPT[] = {
    #include "kalos_idl_compiler.kalos.inc"
};

static const char IDL_COMPILER_IDL[] = {
    #include "kalos_idl_compiler.kidl.inc"
};

static kalos_module_parsed script_modules;
static kalos_module* script_current_module;
static kalos_export* script_current_export;

void kalos_idl_compiler_print(kalos_state state, kalos_string* string) {
    printf("%s", kalos_string_c(state, *string));
}

void kalos_idl_compiler_println(kalos_state state, kalos_string* string) {
    printf("%s\n", kalos_string_c(state, *string));
}

char* function_type_to_string(kalos_function_type type) {
    switch (type) {
        case FUNCTION_TYPE_ANY:
            return "any";
        case FUNCTION_TYPE_BOOL:
            return "bool";
        case FUNCTION_TYPE_NUMBER:
            return "number";
        case FUNCTION_TYPE_OBJECT:
            return "object";
        case FUNCTION_TYPE_STRING:
            return "string";
        case FUNCTION_TYPE_VARARGS:
            return "varargs";
        case FUNCTION_TYPE_VOID:
            return "void";
    }
    return "";
}

static void iter_function_arg(kalos_state state, void* context, uint16_t index, kalos_value* value) {
    value->type = KALOS_VALUE_STRING;
    value->value.string = kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.args[index].type));
}

kalos_string kalos_idl_module_name(kalos_state state) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_module->name_index)); }
kalos_string kalos_idl_module_prefix(kalos_state state) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_module->prefix_index)); }

kalos_int kalos_idl_function_id(kalos_state state) { return script_current_export->entry.function.invoke_id; }
kalos_string kalos_idl_function_return_type(kalos_state state) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.return_type)); }
kalos_string kalos_idl_function_name(kalos_state state) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_export->name_index)); }
kalos_string kalos_idl_function_realname(kalos_state state) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_export->entry.function.symbol_index)); }
kalos_object* kalos_idl_function_args(kalos_state state) { return kalos_allocate_sized_iterable(state, iter_function_arg, 0, NULL, script_current_export->entry.function.arg_count); }
kalos_int kalos_idl_function_arg_count(kalos_state state) { return script_current_export->entry.function.arg_count; }
kalos_string kalos_idl_function_varargs(kalos_state state) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.vararg_type)); }
kalos_string kalos_idl_function_arg_type(kalos_state state, kalos_int arg) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.args[arg].type)); }

kalos_int kalos_idl_property_read_id(kalos_state state) { return script_current_export->entry.property.read_invoke_id; }
kalos_int kalos_idl_property_write_id(kalos_state state) { return script_current_export->entry.property.write_invoke_id; }
kalos_string kalos_idl_property_read_symbol(kalos_state state) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_export->entry.property.read_symbol_index)); }
kalos_string kalos_idl_property_write_symbol(kalos_state state) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_export->entry.property.write_symbol_index)); }
kalos_string kalos_idl_property_type(kalos_state state) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.property.type)); }

#include "kalos_idl_compiler.dispatch.inc"

struct walk_callback_context {
    kalos_module_parsed modules;
    kalos_state state;
};

bool export_walk_callback(void* context_, uint16_t index, kalos_module* module, kalos_export* export) {
    struct walk_callback_context* context = context_;
    script_current_export = export;
    switch (export->type) {
        case KALOS_EXPORT_TYPE_FUNCTION:
            kalos_trigger(context->state, "function");
            break;
        case KALOS_EXPORT_TYPE_PROPERTY:
            kalos_trigger(context->state, "property");
            break;
        default:
            break;
    }
    return true;
}

bool module_walk_callback(void* context_, uint16_t index, kalos_module* module) {
    struct walk_callback_context* context = context_;
    script_current_module = module;
    kalos_trigger(context->state, "begin_module");
    kalos_module_walk_exports(context, context->modules, module, export_walk_callback);
    kalos_trigger(context->state, "end_module");
    return true;
}

void kalos_idl_generate_dispatch(kalos_module_parsed parsed_module) {
    kalos_script script = {0};
    script.script_buffer_size = 4096;
    script.script_ops = malloc(4096);
    kalos_module_parsed modules = kalos_idl_parse_module(IDL_COMPILER_IDL);
    kalos_parse_result result = kalos_parse(IDL_COMPILER_SCRIPT, modules, &script);
    if (result.error) {
        printf("%s\n", result.error);
        exit(1);
    }
    // char* s = malloc(10 * 1024);
    // s[0] = 0;
    // kalos_dump(&script, s);
    // printf("%s", s);
    // free(s);
    kalos_fn fns = {
        malloc,
        free,
        NULL,
    };
    kalos_dispatch_fn dispatch[] = {
        kalos_module_idl_builtin,
        kalos_module_idl_module,
        kalos_module_idl_function,
        kalos_module_idl_property,
    };
    kalos_state state = kalos_init(&script, dispatch, &fns);
    struct walk_callback_context context = { .modules = parsed_module, .state = state };
    script_modules = parsed_module;
    kalos_module_walk_modules(&context, parsed_module, module_walk_callback);
}
