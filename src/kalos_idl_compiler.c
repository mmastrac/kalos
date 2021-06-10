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
    size_t object_props_count;
    size_t export_count;
    size_t function_index;
    size_t handle_index;
    bool in_object;
};

static void* export_compare_context;
int export_compare(const void* v1, const void* v2) {
    struct kalos_module_builder* builder = export_compare_context;
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
        if (s[i] == '\\') {
            i++;
            switch (s[i]) {
                case '\\': s[out] = '\\'; break;
                case 'r': s[out] = '\r'; break;
                case 'n': s[out] = '\n'; break;
                case 't': s[out] = '\t'; break;
                case 'x': {
                    char n[3];
                    if (i >= len - 3) {
                        return NULL;
                    }
                    n[0] = s[++i];
                    n[1] = s[++i];
                    n[2] = 0;
                    char* e;
                    s[out] = (char)strtol(n, &e, 16);
                    if (e != n + 2) {
                        return NULL;
                    }
                    break;
                }
                default: return NULL;
            }
        } else {
            s[out] = s[i];
        }
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

static void error(void* context, const char* start, uint16_t error_pos) {
    uint16_t row = 0, col = 0;
    for (int i = 0; i < error_pos; i++) {
        if (start[i] == '\n') {
            row++;
            col = 0;
        } else {
            col++;
        }
    }
    printf("Error on line %d, column %d\n", row + 1, col + 1);
}

static bool prefix(void* context, const char* prefix) {
    struct kalos_module_builder* builder = context;
    prefix = unstring((char*)prefix);
    if (!prefix) {
        return false;
    }
    builder->prefix_index = strpack(builder, prefix);
    return true;
}

static void begin_module(void* context, const char* module) {
    struct kalos_module_builder* builder = context;
    builder->export_count = 0;
    builder->module_count++;
    builder->module_name_index = strpack(builder, module);
    builder->function_index = 1;
    builder->handle_index = 1;
    LOG("module %s", module);
}

static void end_module(void* context) {
    struct kalos_module_builder* builder = context;
    export_compare_context = context;
    qsort(builder->exports, builder->export_count, sizeof(kalos_export), export_compare);
    export_compare_context = NULL;
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

static void begin_object(void* context, const char* name) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name_index = strpack(builder, name);
    current_export(builder)->type = KALOS_EXPORT_TYPE_OBJECT;
    builder->in_object = true;
}

static void end_object(void* context) {
    struct kalos_module_builder* builder = context;
    builder->in_object = false;
}

static void begin_function(void* context, const char* name) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
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

static void begin_handle(void* context, const char* name) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name_index = strpack(builder, name);
    current_export(builder)->type = KALOS_EXPORT_TYPE_HANDLE;
    current_export(builder)->entry.function.invoke_id = builder->handle_index++;
    LOG("handle %s", name);
}

static void handle_arg(void* context, const char* name, const char* type, bool is_varargs) {
    struct kalos_module_builder* builder = context;
    int arg = current_export(builder)->entry.function.arg_count++;
    current_export(builder)->entry.function.args[arg].name_index = strpack(builder, name);
    current_export(builder)->entry.function.args[arg].type = strtotype(type);
    LOG("arg %s %s", name, type);
}

static void end_handle(void* context) {
    LOG("handle");
}

static bool constant_string(void* context, const char* name, const char* type, const char* s) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name_index = strpack(builder, name);
    current_export(builder)->type = KALOS_EXPORT_TYPE_CONST_STRING;
    s = unstring((char*)s);
    if (!s) {
        return false;
    }
    current_export(builder)->entry.const_string_index = strpack(builder, s);
    LOG("const %s %s %s", name, type, s);
    return true;
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
    kalos_property* property;
    if (builder->in_object) {
        int prop = current_export(builder)->entry.object.property_count++;
        property = &current_export(builder)->entry.object.properties[prop].property;
        current_export(builder)->entry.object.properties[prop].name_index = strpack(builder, name);
        builder->object_props_count++;
    } else {
        new_export(builder);
        current_export(builder)->name_index = strpack(builder, name);
        current_export(builder)->type = KALOS_EXPORT_TYPE_PROPERTY;
        property = &current_export(builder)->entry.property;
    }
    property->type = strtotype(type);
    if (strcmp(mode, "read") == 0) {
        property->read_invoke_id = builder->in_object ? 1 : builder->function_index++;
        property->read_symbol_index = strpack(builder, symbol);
    } else if (strcmp(mode, "write") == 0) {
        property->write_invoke_id = builder->in_object ? 1 : builder->function_index++;
        property->write_symbol_index = strpack(builder, symbol);
    } else if (strcmp(mode, "read,write") == 0) {
        property->read_invoke_id = builder->in_object ? 1 : builder->function_index++;
        property->write_invoke_id = builder->in_object ? 1 : builder->function_index++;
        property->read_symbol_index = strpack(builder, symbol);
        property->write_symbol_index = strpack(builder, symbol2);
    }
    LOG("prop %s %s %s %s %s", name, type, mode, symbol, symbol2);
}

static void* property_compare_context;
int property_compare(const void* v1, const void* v2) {
    kalos_module_parsed* parsed = property_compare_context;
    kalos_object_property** e1 = (kalos_object_property**)v1;
    kalos_object_property** e2 = (kalos_object_property**)v2;

    int res = strcmp(kalos_module_get_string(*parsed, (*e1)->name_index), kalos_module_get_string(*parsed, (*e2)->name_index));
    if (res != 0) {
        return res;
    }

    return (int)(*e1)->property.type - (int)(*e2)->property.type;
}

bool object_prop_export_callback(void* context, kalos_module_parsed parsed, uint16_t index, kalos_module* module, kalos_export* export) {
    kalos_object_property*** object_props = context;
    if (export->type == KALOS_EXPORT_TYPE_OBJECT) {
        for (int i = 0; i < export->entry.object.property_count; i++) {
            **object_props = &export->entry.object.properties[i];
            (*object_props)++;
        }
    }
    return true;
}

bool object_prop_module_callback(void* context, kalos_module_parsed parsed, uint16_t index, kalos_module* module) {
    kalos_module_walk_exports(context, parsed, module, object_prop_export_callback);
    return true;
}

kalos_module_parsed kalos_idl_parse_module(const char* s) {
    kalos_idl_callbacks callbacks = {
        error,
        prefix,
        begin_module,
        end_module,
        begin_object,
        end_object,
        begin_function,
        function_arg,
        end_function,
        begin_handle,
        handle_arg,
        end_handle,
        constant_string,
        constant_number,
        property
    };

    struct kalos_module_builder context = {0};
    context.prefix_index = -1;

    if (!kalos_idl_parse_callback(s, &context, &callbacks)) {
        kalos_module_parsed parsed = {0};
        return parsed;
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

    kalos_module_parsed parsed;
    parsed.data=context.kalos_module_buffer;
    parsed.size=sizeof(kalos_module_header) + context.module_buffer_size + context.string_buffer_index;

    // Post-process object properties
    kalos_object_property** object_props = malloc(sizeof(kalos_object_property*) * context.object_props_count);
    kalos_object_property** object_props_ptr = object_props;
    kalos_module_walk_modules(&object_props_ptr, parsed, object_prop_module_callback);
    property_compare_context = &parsed;
    qsort(object_props, context.object_props_count, sizeof(object_props[0]), property_compare);
    property_compare_context = NULL;
    kalos_int module_prop_index = 0;
    const char* current_name = "";
    kalos_function_type current_type;
    kalos_property_address* prop_addrs = NULL;
    int prop_addr_count = 0;
    for (int i = 0; i < context.object_props_count; i++) {
        kalos_object_property* p = object_props[i];
        // TODO: Typing will require us to split props by type
        if (strcmp(kalos_module_get_string(parsed, p->name_index), current_name) != 0/* || current_type != p->property.type*/) {
            module_prop_index++;
            current_name = kalos_module_get_string(parsed, p->name_index);
            current_type = p->property.type;
            prop_addr_count++;
            prop_addrs = realloc(prop_addrs, prop_addr_count * sizeof(prop_addrs[0]));
            prop_addrs[prop_addr_count - 1].name_index = p->name_index;
            prop_addrs[prop_addr_count - 1].type = p->property.type;
        }
        p->property.read_invoke_id = p->property.read_invoke_id ? module_prop_index * 2 : 0;
        p->property.write_invoke_id = p->property.write_invoke_id ? module_prop_index * 2 + 1 : 0;
    }
    header->props_offset = parsed.size;
    header->props_count = prop_addr_count;
    parsed.data = realloc(parsed.data, parsed.size + prop_addr_count * sizeof(prop_addrs[0]));
    memcpy((uint8_t*)parsed.data + parsed.size, prop_addrs, prop_addr_count * sizeof(prop_addrs[0]));
    parsed.size += prop_addr_count * sizeof(prop_addrs[0]);

    return parsed;
}

static kalos_printer_fn script_output;
static kalos_module_parsed script_modules;
static kalos_module* script_current_module;
static kalos_export* script_current_export;
static kalos_object_property* script_current_property;

void kalos_idl_compiler_print(kalos_state state, kalos_string* string) {
    script_output("%s", kalos_string_c(state, *string));
}

void kalos_idl_compiler_println(kalos_state state, kalos_string* string) {
    script_output("%s\n", kalos_string_c(state, *string));
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
kalos_string kalos_idl_module_prefix(kalos_state state) { return kalos_string_allocate(state, script_current_module->prefix_index != -1 ? kalos_module_get_string(script_modules, script_current_module->prefix_index) : "kalos_idl_"); }
kalos_string kalos_idl_obj_module_name(kalos_state state, kalos_object* object) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_module->name_index)); }
kalos_string kalos_idl_obj_module_prefix(kalos_state state, kalos_object* object) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_module->prefix_index)); }
kalos_string kalos_idl_export_name(kalos_state state) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_export->name_index)); }

kalos_int kalos_idl_function_id(kalos_state state) { return script_current_export->entry.function.invoke_id; }
kalos_string kalos_idl_function_return_type(kalos_state state) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.return_type)); }
kalos_string kalos_idl_function_realname(kalos_state state) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_export->entry.function.symbol_index)); }
kalos_object* kalos_idl_function_args(kalos_state state) { return kalos_allocate_sized_iterable(state, iter_function_arg, 0, NULL, script_current_export->entry.function.arg_count); }
kalos_int kalos_idl_function_arg_count(kalos_state state) { return script_current_export->entry.function.arg_count; }
kalos_string kalos_idl_function_varargs(kalos_state state) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.vararg_type)); }
kalos_string kalos_idl_function_arg_type(kalos_state state, kalos_int arg) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.args[arg].type)); }

kalos_string kalos_idl_export_name2(kalos_state state, kalos_object* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_export->name_index)); }
kalos_int kalos_idl_function_id2(kalos_state state, kalos_object* o) { return script_current_export->entry.function.invoke_id; }
kalos_string kalos_idl_function_return_type2(kalos_state state, kalos_object* o) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.return_type)); }
kalos_string kalos_idl_function_realname2(kalos_state state, kalos_object* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_export->entry.function.symbol_index)); }
kalos_object* kalos_idl_function_args2(kalos_state state, kalos_object* o) { return kalos_allocate_sized_iterable(state, iter_function_arg, 0, NULL, script_current_export->entry.function.arg_count); }
kalos_int kalos_idl_function_arg_count2(kalos_state state, kalos_object* o) { return script_current_export->entry.function.arg_count; }
kalos_string kalos_idl_function_varargs2(kalos_state state, kalos_object* o) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.vararg_type)); }
kalos_string kalos_idl_function_arg_type2(kalos_state state, kalos_object* o, kalos_int arg) { return kalos_string_allocate(state, function_type_to_string(script_current_export->entry.function.args[arg].type)); }

kalos_property* prop() { return script_current_property ? &script_current_property->property : &script_current_export->entry.property; }

kalos_string kalos_idl_property_name(kalos_state state) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_property ? script_current_property->name_index : script_current_export->name_index)); }
kalos_int kalos_idl_property_read_id(kalos_state state) { return prop()->read_invoke_id; }
kalos_int kalos_idl_property_write_id(kalos_state state) { return prop()->write_invoke_id; }
kalos_string kalos_idl_property_read_symbol(kalos_state state) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, prop()->read_symbol_index)); }
kalos_string kalos_idl_property_write_symbol(kalos_state state) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, prop()->write_symbol_index)); }
kalos_string kalos_idl_property_type(kalos_state state) { return kalos_string_allocate(state, function_type_to_string(prop()->type)); }

kalos_string kalos_idl_property_name2(kalos_state state, kalos_object* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_property ? script_current_property->name_index : script_current_export->name_index)); }
kalos_int kalos_idl_property_read_id2(kalos_state state, kalos_object* o) { return prop()->read_invoke_id; }
kalos_int kalos_idl_property_write_id2(kalos_state state, kalos_object* o) { return prop()->write_invoke_id; }
kalos_string kalos_idl_property_read_symbol2(kalos_state state, kalos_object* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, prop()->read_symbol_index)); }
kalos_string kalos_idl_property_write_symbol2(kalos_state state, kalos_object* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, prop()->write_symbol_index)); }
kalos_string kalos_idl_property_type2(kalos_state state, kalos_object* o) { return kalos_string_allocate(state, function_type_to_string(prop()->type)); }

kalos_int kalos_idl_handle_index(kalos_state state) { return script_current_export->entry.function.invoke_id; }
kalos_int kalos_idl_handle_module_index(kalos_state state) { return script_current_module->index; }

struct walk_callback_context {
    bool handles;
    kalos_module_parsed modules;
    kalos_state state;
    kalos_value script_context;
};

bool export_walk_callback(void* context_, kalos_module_parsed parsed, uint16_t index, kalos_module* module, kalos_export* export);
bool module_walk_callback(void* context_, kalos_module_parsed parsed, uint16_t index, kalos_module* module);

void kalos_idl_walk_modules(kalos_state state, kalos_value* script_context) {
    struct walk_callback_context context;
    context.state = state;
    kalos_value_move_to(state, script_context, &context.script_context);
    context.modules = script_modules;
    kalos_module_walk_modules(&context, script_modules, module_walk_callback);
    kalos_clear(state, &context.script_context);
}

void kalos_idl_walk_exports(kalos_state state, kalos_value* script_context) {
    struct walk_callback_context context;
    context.state = state;
    kalos_value_move_to(state, script_context, &context.script_context);
    context.modules = script_modules;
    kalos_module_walk_exports(&context, script_modules, script_current_module, export_walk_callback);
    kalos_clear(state, &context.script_context);
}

#include "kalos_idl_compiler.dispatch.inc"

bool export_walk_callback(void* context_, kalos_module_parsed parsed, uint16_t index, kalos_module* module, kalos_export* export) {
    struct walk_callback_context* context = context_;
    script_current_export = export;
    switch (export->type) {
        case KALOS_EXPORT_TYPE_FUNCTION: {
            kalos_value v = kalos_value_clone(context->state, &context->script_context);
            // kalos_module_idl_module_trigger_function(context->state, &v, NULL);
            break;
        }
        case KALOS_EXPORT_TYPE_PROPERTY:
            // kalos_module_idl_trigger_property_export(context->state);
            break;
        case KALOS_EXPORT_TYPE_HANDLE:
            // kalos_module_idl_trigger_handle_export(context->state);
            break;
        case KALOS_EXPORT_TYPE_OBJECT:
            break;
        default:
            break;
    }
    return true;
}

bool export_walk_callback2(void* context_, kalos_module_parsed parsed, uint16_t index, kalos_module* module, kalos_export* export) {
    struct walk_callback_context* context = context_;
    script_current_export = export;
    switch (export->type) {
        case KALOS_EXPORT_TYPE_FUNCTION:
            if (!context->handles) {
                kalos_module_idl_module_trigger_function(context->state, &context->script_context, kalos_allocate_prop_object(context->state, NULL, kalos_module_idl_module_object_function_props));
            }
            break;
        case KALOS_EXPORT_TYPE_PROPERTY:
            if (!context->handles) {
                kalos_module_idl_module_trigger_property(context->state, &context->script_context, kalos_allocate_prop_object(context->state, NULL, kalos_module_idl_module_object_property_props));
            }
            break;
        case KALOS_EXPORT_TYPE_HANDLE:
            if (context->handles) {
                kalos_module_idl_module_trigger_handle_(context->state, &context->script_context, kalos_allocate_prop_object(context->state, NULL, kalos_module_idl_module_object_property_props));
            }
            break;
        case KALOS_EXPORT_TYPE_OBJECT:
            if (context->handles) {
                kalos_module_idl_trigger_begin_object(context->state);
                for (int i = 0; i < export->entry.object.property_count; i++) {
                    script_current_property = &export->entry.object.properties[i];
                    kalos_module_idl_module_trigger_property(context->state, &context->script_context, kalos_allocate_prop_object(context->state, NULL, kalos_module_idl_module_object_property_props));
                }
                script_current_property = NULL;
                kalos_module_idl_trigger_end_object(context->state);
            }
            break;
        default:
            break;
    }
    return true;
}

bool module_walk_callback(void* context_, kalos_module_parsed parsed, uint16_t index, kalos_module* module) {
    struct walk_callback_context* context = context_;
    script_current_module = module;
    context->handles = true;
    kalos_module_walk_exports(context, parsed, module, export_walk_callback2);
    kalos_value ctx = kalos_value_clone(context->state, &context->script_context);
    kalos_module_idl_module_trigger_begin(context->state, &ctx, kalos_allocate_prop_object(context->state, module, kalos_module_idl_module_object_module_props));
    context->handles = false;
    kalos_module_walk_exports(context, parsed, module, export_walk_callback2);
    kalos_module_idl_module_trigger_end(context->state);
    return true;
}

bool kalos_idl_generate_dispatch(kalos_module_parsed parsed_module, kalos_printer_fn output) {
    // TODO: Watcom takes forever to compile if these aren't static
    static const char IDL_COMPILER_SCRIPT[] = {
        #include "kalos_idl_compiler.kalos.inc"
    };

    static const char IDL_COMPILER_IDL[] = {
        #include "kalos_idl_compiler.kidl.inc"
    };
    script_output = output;
    kalos_script script = {0};
    script.script_buffer_size = 8192;
    script.script_ops = malloc(8192);
    kalos_module_parsed modules = kalos_idl_parse_module(IDL_COMPILER_IDL);
    if (!modules.data) {
        printf("ERROR: %s\n", "failed to parse compiler IDL");
        return false;
    }
    kalos_parse_result result = kalos_parse(IDL_COMPILER_SCRIPT, modules, &script);
    if (result.error) {
        printf("ERROR: %s\n", result.error);
        return false;
    }
    // char* s = malloc(30 * 1024);
    // s[0] = 0;
    // kalos_dump(&script, s);
    // printf("%s", s);
    // free(s);
    kalos_fn fns = {
        malloc,
        free,
        NULL
    };
    script_modules = parsed_module;
    kalos_state state = kalos_init(&script, kalos_module_idl_dispatch, &fns);
    kalos_module_idl_trigger_open(state);
    struct walk_callback_context context;
    context.modules = parsed_module;
    context.state = state;
    // kalos_module_walk_modules(&context, parsed_module, module_walk_callback);
    kalos_module_idl_trigger_close(state);
    return true;
}
