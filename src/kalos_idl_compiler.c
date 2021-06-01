#include <stdbool.h>

#include "kalos_module.h"
#include "kalos_parse.h"
#include "kalos_run.h"
#include "kalos_idl_compiler.h"
#include "kalos_idl_parse.h"

struct kalos_module_builder {
    void* kalos_module_buffer;
    size_t module_buffer_size;
    size_t module_count;
    const char* module_name;
    char* string_buffer;
    size_t string_buffer_size;
    size_t string_buffer_index;
    kalos_export* exports;
    size_t export_count;
    size_t function_arg_count;
};

typedef struct kalos_module_header {
    kalos_int version;
    kalos_int module_count;
    kalos_int module_offset;
    kalos_int module_size;
    kalos_int string_offset;
    kalos_int string_size;
    kalos_int unused1;
    kalos_int unused2;
} kalos_module_header;

int export_compare(void* context, const void* v1, const void* v2) {
    struct kalos_module_builder* builder = context;
    const kalos_export* e1 = v1;
    const kalos_export* e2 = v2;
    int cmp = strcmp((int)e1->name + builder->string_buffer, (int)e2->name + builder->string_buffer);
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

static const char* strpack(struct kalos_module_builder* builder, const char* s) {
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
    return (const char*)(builder->string_buffer_index - l); // offset into buffer
}

static inline void strunpack(const char* strings, const char** s) {
    *s = (strings + (off_t)*s);
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
    struct kalos_module_builder* builder = context;
    builder->export_count = 0;
    builder->module_count++;
    builder->module_name = strpack(builder, module);
    LOG("module %s", module);
}

static void end_module(void* context) {
    struct kalos_module_builder* builder = context;
    qsort_r(builder->exports, builder->export_count, sizeof(kalos_export), context, export_compare);
    size_t this_module_size = sizeof(kalos_module) + sizeof(kalos_export) * builder->export_count;
    builder->module_buffer_size += this_module_size;
    builder->kalos_module_buffer = realloc(builder->kalos_module_buffer, builder->module_buffer_size);

    kalos_module* module = (kalos_module*)((uint8_t*)builder->kalos_module_buffer + builder->module_buffer_size - this_module_size);
    module->name = builder->module_name;
    module->export_count = builder->export_count;
    memcpy((uint8_t*)module + sizeof(kalos_module), builder->exports, builder->export_count * sizeof(kalos_export));
    free(builder->exports);
    builder->exports = 0;
    LOG("end");
}

static void begin_function(void* context, const char* name) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->type = KALOS_EXPORT_TYPE_FUNCTION;
    current_export(builder)->name = strpack(builder, name);
    current_export(builder)->entry.function.vararg_type = FUNCTION_TYPE_VOID;
    LOG("fn %s", name);
}

static void function_arg(void* context, const char* name, const char* type, bool is_varargs) {
    struct kalos_module_builder* builder = context;
    if (is_varargs) {
        current_export(builder)->entry.function.vararg_type = strtotype(type);
    } else {
        int arg = current_export(builder)->entry.function.arg_count++;
        current_export(builder)->entry.function.args[arg].name = strpack(builder, name);
        current_export(builder)->entry.function.args[arg].type = strtotype(type);
    }
    LOG("arg %s %s", name, type);
}

static void end_function(void* context, const char* name, const char* type, const char* symbol) {
    struct kalos_module_builder* builder = context;
    current_export(builder)->entry.function.return_type = strtotype(type);
    current_export(builder)->entry.function.symbol = strpack(builder, symbol);
    LOG("fn %s %s %s", name, type, symbol);
}

static void constant_string(void* context, const char* name, const char* type, const char* s) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name = strpack(builder, name);
    current_export(builder)->entry.const_string = strpack(builder, s);
    LOG("const %s %s %s", name, type, s);
}

static void constant_number(void* context, const char* name, const char* type, kalos_int number) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name = strpack(builder, name);
    current_export(builder)->type = KALOS_EXPORT_TYPE_CONST_NUMBER;
    current_export(builder)->entry.const_number = number;
    LOG("const %s %s %d", name, type, number);
}

static void property(void* context, const char* name, const char* type, const char* mode, const char* symbol) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name = strpack(builder, name);
    current_export(builder)->entry.function.symbol = strpack(builder, symbol);
    current_export(builder)->entry.function.return_type = strtotype(type);
    if (strcmp(mode, "read") == 0) {
        current_export(builder)->type = KALOS_EXPORT_TYPE_PROP_READ;
    } else if (strcmp(mode, "write") == 0) {
        current_export(builder)->type = KALOS_EXPORT_TYPE_PROP_WRITE;
    }
    LOG("prop %s %s %s %s", name, type, mode, symbol);
}

kalos_module_parsed kalos_idl_parse_module(const char* s) {
    kalos_idl_callbacks callbacks = {
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

kalos_module** kalos_idl_unpack_module(uint8_t* packed_module) {
    kalos_module_header* header = (kalos_module_header*)packed_module;

    kalos_module** output = malloc(sizeof(kalos_module*) * (header->module_count + 1));
    kalos_module* m = (void*)(packed_module + header->module_offset);
    const char* strings = (const char*)packed_module + header->string_offset;

    for (int i = 0; i < header->module_count; i++) {
        output[i] = m;
        strunpack(strings, &m->name);
        kalos_export* e = (kalos_export*)((uint8_t*)m + sizeof(kalos_module));
        m->exports = e;
        for (int j = 0; j < m->export_count; j++) {
            strunpack(strings, &e[j].name);
            switch (e[j].type) {
                case KALOS_EXPORT_TYPE_CONST_STRING:
                    strunpack(strings, &e[j].entry.const_string);
                    break;
                case KALOS_EXPORT_TYPE_FUNCTION:
                case KALOS_EXPORT_TYPE_PROP_READ:
                case KALOS_EXPORT_TYPE_PROP_WRITE:
                    strunpack(strings, &e[j].entry.function.symbol);
                    break;
                default:
                    break;
            }
        }
        m = (void*)((uint8_t*)m + sizeof(kalos_module) + sizeof(kalos_export) * m->export_count);
    }
    output[header->module_count] = NULL;
    return output;
}

static const char IDL_COMPILER_SCRIPT[] = {
    #include "kalos_idl_compiler.kalos.inc"
    , 0
};

static const char IDL_COMPILER_IDL[] = {
    #include "kalos_idl_compiler.idl.inc"
    , 0
};

static kalos_module* script_current_module;
static kalos_export* script_current_export;
static kalos_int script_current_fn_index;

void kalos_idl_compiler_print(kalos_state state, kalos_string* string) {
    printf("%s", kalos_string_c(state, *string));
}

void kalos_idl_compiler_println(kalos_state state, kalos_string* string) {
    printf("%s\n", kalos_string_c(state, *string));
}

kalos_string kalos_idl_module_name(kalos_state state) {
    return kalos_string_allocate(state, script_current_module->name);
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

kalos_int kalos_idl_function_id(kalos_state state) { return script_current_fn_index; }
kalos_string kalos_idl_function_type(kalos_state state) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.return_type)); }
kalos_string kalos_idl_function_return_type(kalos_state state) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.return_type)); }
kalos_string kalos_idl_function_name(kalos_state state) { return kalos_string_allocate(state, script_current_export->name); }
kalos_string kalos_idl_function_realname(kalos_state state) { return kalos_string_allocate(state, script_current_export->entry.function.symbol); }
kalos_object* kalos_idl_function_args(kalos_state state) { return kalos_allocate_sized_iterable(state, iter_function_arg, 0, NULL, script_current_export->entry.function.arg_count); }
kalos_int kalos_idl_function_arg_count(kalos_state state) { return script_current_export->entry.function.arg_count; }
kalos_string kalos_idl_function_varargs(kalos_state state) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.vararg_type)); }
kalos_string kalos_idl_function_arg_type(kalos_state state, kalos_int arg) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.args[arg].type)); }

void dispatch_function(kalos_state state, int function, kalos_stack* stack) {
    switch (function) {
        case 0:
            push_number(stack, script_current_export->entry.function.arg_count);
            break;
        case 1:
            push_string(stack, kalos_string_allocate(state, "void"));
            break;
        case 2:
            // TODO: This should use a context, but we'll assume only one of these are in flight at any time
            push_object(stack, kalos_allocate_sized_iterable(state, iter_function_arg, 0, NULL, script_current_export->entry.function.arg_count));
            break;
        case 3:
            push_number(stack, script_current_fn_index);
            break;
        case 4:
            push_string(stack, kalos_string_allocate(state, script_current_export->name));
            break;
        case 5:
            push_string(stack, kalos_string_allocate(state, script_current_export->entry.function.symbol));
            break;
        case 6:
            push_string(stack, kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.return_type)));
            break;
        case 7:
            break;
        case 8:
            push_string(stack, kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.vararg_type)));
            break;
    }
}

#include "kalos_idl_compiler_dispatch.inc"

void kalos_idl_generate_dispatch(kalos_module_parsed parsed_module) {
    kalos_module** modules = kalos_idl_unpack_module(parsed_module.data);
    kalos_script script = {0};
    script.script_buffer_size = 2048;
    script.script_ops = malloc(2048);
    kalos_module_parsed parsed = kalos_idl_parse_module(IDL_COMPILER_IDL);
    kalos_module** modules_for_script = kalos_idl_unpack_module(parsed.data);
    kalos_parse(IDL_COMPILER_SCRIPT, modules_for_script, &script);
    kalos_fn fns = {
        malloc,
        free,
        NULL,
    };
    kalos_dispatch_fn dispatch[] = {
        kalos_module_dispatch_builtin,
        kalos_module_dispatch_module,
        kalos_module_dispatch_function,
    };
    kalos_state state = kalos_init(&script, dispatch, &fns);
    for (int i = 0;; i++) {
        if (!modules[i]) {
            break;
        }
        script_current_module = modules[i];
        kalos_trigger(state, "begin_module");
        for (int j = 0; j < modules[i]->export_count; j++) {
            if (modules[i]->exports[j].type != KALOS_EXPORT_TYPE_FUNCTION 
                && modules[i]->exports[j].type != KALOS_EXPORT_TYPE_PROP_READ
                && modules[i]->exports[j].type != KALOS_EXPORT_TYPE_PROP_WRITE) {
                continue;
            }
            script_current_export = &modules[i]->exports[j];
            script_current_fn_index = j;
            kalos_trigger(state, "function");
        }
        kalos_trigger(state, "end_module");
    }
}
