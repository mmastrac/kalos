#include <stdbool.h>
#include "defines.h"
#include "kalos.h"
#include "kalos_dump.h"
#include "kalos_module.h"
#include "kalos_parse.h"
#include "kalos_run.h"
#include "kalos_idl_compiler.h"
#include "kalos_idl_parse.h"
#include "kalos_util.h"

struct kalos_module_builder {
    kalos_basic_environment* env;
    void* kalos_module_buffer;
    size_t module_buffer_size, module_buffer_index;
    char* string_buffer;
    size_t string_buffer_size;
    size_t string_buffer_index;
    bool in_object;
    kalos_int handle_index;
    kalos_int function_index;
};

static void* allocate_item(struct kalos_module_builder* builder, kalos_module_item_list_header** list, size_t struct_size) {
    if (builder->module_buffer_index + struct_size > builder->module_buffer_size) {
        ptrdiff_t list_offset = PTR_BYTE_SUBTRACT(*list, builder->kalos_module_buffer);
        builder->module_buffer_size *= 2;
        builder->kalos_module_buffer = builder->env->realloc(builder->kalos_module_buffer, builder->module_buffer_size);
        (*list) = PTR_BYTE_OFFSET(builder->kalos_module_buffer, list_offset);
    }
    void* ptr = PTR_BYTE_OFFSET(builder->kalos_module_buffer, builder->module_buffer_index);
    memset(ptr, 0, struct_size);
    return ptr;
}
 
static void* append_list_item(struct kalos_module_builder* builder, kalos_module_item_list_header* list, size_t struct_size) {
    void* ptr = allocate_item(builder, &list, struct_size);
    if (list->count == 0) {
        list->head = list->tail = builder->module_buffer_index;
        kalos_module_item_list* item = PTR_BYTE_OFFSET(builder->kalos_module_buffer, list->tail);
        item->next = item->prev = 0;
    } else {
        kalos_module_item_list* item = PTR_BYTE_OFFSET(builder->kalos_module_buffer, list->tail);
        item->next = builder->module_buffer_index;
        item = ptr;
        item->prev = list->tail;
        item->next = 0;
        list->tail = builder->module_buffer_index;
    }
    list->count++;
    builder->module_buffer_index += struct_size;
    return ptr;
}

typedef int (*list_find_fn)(void* context, void* a, void* b);

static void* insert_list_item(struct kalos_module_builder* builder, kalos_module_item_list_header* list, size_t struct_size, void* context, void* a, list_find_fn fn) {
    if (list->count > 0) {
        void* ptr = allocate_item(builder, &list, struct_size);
        kalos_int item_offset = list->head;
        while (item_offset) {
            kalos_module_item_list* item = PTR_BYTE_OFFSET(builder->kalos_module_buffer, item_offset);
            int compare = fn(context, a, item);
            if (compare == 0) {
                return item; // Exact match
            } else if (compare == -1) {
                // Insert before here
                list->count++;
                kalos_module_item_list* new_item = ptr;
                new_item->prev = item->prev;
                new_item->next = item_offset;
                kalos_int prev_item_offset = item->prev;
                item->prev = builder->module_buffer_index;
                if (prev_item_offset == 0) {
                    list->head = builder->module_buffer_index;
                } else {
                    item = PTR_BYTE_OFFSET(builder->kalos_module_buffer, prev_item_offset);
                    item->next = builder->module_buffer_index;
                }
                builder->module_buffer_index += struct_size;
                return ptr;
            }
            item_offset = item->next;
        }
    }
    return append_list_item(builder, list, struct_size);
}

// Finds either the insertion point for a new item, or the existing item that matches
static void* find_list_item(struct kalos_module_builder* builder, kalos_module_item_list_header* list, void* context, void* a, list_find_fn fn) {
    return NULL;
}

static void* get_list_item(struct kalos_module_builder* builder, kalos_int offset) {
    ASSERT(offset > 0);
    return PTR_BYTE_OFFSET(builder->kalos_module_buffer, offset);
}

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

struct kalos_module_header* root(struct kalos_module_builder* builder) {
    return builder->kalos_module_buffer;
}

static void new_export(struct kalos_module_builder* builder) {
    kalos_module* module = get_list_item(builder, root(builder)->module_list.tail);
    append_list_item(builder, &module->export_list, sizeof(kalos_export));
}

static kalos_export* current_export(struct kalos_module_builder* builder) {
    kalos_module* module = get_list_item(builder, root(builder)->module_list.tail);
    return get_list_item(builder, module->export_list.tail);
}

static kalos_int strpack(struct kalos_module_builder* builder, const char* s) {
    size_t l = strlen(s) + 1;
    while (builder->string_buffer_size - builder->string_buffer_index < l) {
        if (builder->string_buffer_size == 0) {
            builder->string_buffer_size = 16;
        }
        builder->string_buffer_size *= 2;
        builder->string_buffer = builder->env->realloc(builder->string_buffer, builder->string_buffer_size);
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
    LOG("prefix %s", prefix);
    root(builder)->prefix_index = strpack(builder, prefix);
    return true;
}

static void dispatch_name(void* context) {
    struct kalos_module_builder* builder = context;
    root(builder)->flags |= KALOS_MODULE_FLAG_DISPATCH_NAME;
    LOG("dispatch name");
}

static int insert_module_fn(void* context, void* a, void* b) {
    struct kalos_module_builder* builder = context;
    kalos_module* module = b;
    return strcmp((const char*)a, builder->string_buffer + module->name_index);
}

static void begin_module(void* context, const char* module) {
    struct kalos_module_builder* builder = context;
    kalos_module* m = insert_list_item(builder, &root(builder)->module_list, sizeof(kalos_module), builder, (void*)module, insert_module_fn);
    // kalos_module* m = append_list_item(builder, &root(builder)->module_list, sizeof(kalos_module));
    m->name_index = strpack(builder, module);
    m->index = root(builder)->module_list.count - 1;
    builder->function_index = 0;
    builder->handle_index = 0;
    LOG("module %s", module);
}

static void end_module(void* context) {
    LOG("/module");
}

static void begin_object(void* context, const char* name) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name_index = strpack(builder, name);
    current_export(builder)->type = KALOS_EXPORT_TYPE_OBJECT;
    builder->in_object = true;
    LOG("object %s", name);
}

static void end_object(void* context) {
    struct kalos_module_builder* builder = context;
    builder->in_object = false;
    LOG("/object");
}

static void begin_function(void* context, const char* name) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->type = KALOS_EXPORT_TYPE_FUNCTION;
    current_export(builder)->name_index = strpack(builder, name);
    kalos_function* fn = append_list_item(builder, &current_export(builder)->entry.function_overload_list, sizeof(kalos_function));
    fn->invoke_id = ++builder->function_index;
    fn->vararg_type = FUNCTION_TYPE_VOID;
    LOG("fn %s (invoke=%d)", name, fn->invoke_id);
}

static void function_arg(void* context, const char* name, const char* type, bool is_varargs) {
    struct kalos_module_builder* builder = context;
    kalos_function* fn = get_list_item(builder, current_export(builder)->entry.function_overload_list.tail);
    if (is_varargs) {
        fn->vararg_type = strtotype(type);
    } else {
        kalos_arg* arg = append_list_item(builder, &fn->arg_list, sizeof(kalos_arg));
        arg->name_index = strpack(builder, name);
        arg->type = strtotype(type);
    }
    LOG("arg %s %s", name, type);
}

static void end_function(void* context, const char* name, const char* type, const char* symbol) {
    struct kalos_module_builder* builder = context;
    kalos_function* fn = get_list_item(builder, current_export(builder)->entry.function_overload_list.tail);
    fn->return_type = strtotype(type);
    fn->symbol_index = strpack(builder, symbol);
    LOG("/fn %s %s %s", name, type, symbol);
}

static void end_function_c(void* context, const char* name, const char* type, const char* c) {
    struct kalos_module_builder* builder = context;
    kalos_function* fn = get_list_item(builder, current_export(builder)->entry.function_overload_list.tail);
    fn->return_type = strtotype(type);
    fn->c_index = strpack(builder, unstring((char*)c));
    LOG("/fn %s %s %s", name, type, c);
}

static void begin_handle(void* context, const char* name) {
    struct kalos_module_builder* builder = context;
    new_export(builder);
    current_export(builder)->name_index = strpack(builder, name);
    current_export(builder)->type = KALOS_EXPORT_TYPE_HANDLE;
    current_export(builder)->entry.handler.invoke_id = ++builder->handle_index;
    LOG("handle %s", name);
}

static void handle_arg(void* context, const char* name, const char* type, bool is_varargs) {
    struct kalos_module_builder* builder = context;
    kalos_arg* arg = append_list_item(builder, &current_export(builder)->entry.handler.arg_list, sizeof(kalos_arg));
    arg->name_index = strpack(builder, name);
    arg->type = strtotype(type);
    LOG("arg %s %s", name, type);
}

static void end_handle(void* context) {
    LOG("/handle");
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

static int insert_prop_addr_fn(void* context, void* a, void* b) {
    struct kalos_module_builder* builder = context;
    kalos_property_address* prop_addr = b;
    return strcmp((const char*)a, builder->string_buffer + prop_addr->name_index);
}

static void property(void* context, const char* name, const char* type, const char* mode, const char* symbol, const char* symbol2) {
    struct kalos_module_builder* builder = context;
    kalos_property* property;
    if (builder->in_object) {
        kalos_int name_index = strpack(builder, name);
        kalos_property_address* obj_prop_addr = insert_list_item(builder, &root(builder)->prop_list, sizeof(kalos_property_address), builder, (void*)name, insert_prop_addr_fn);
        obj_prop_addr->name_index = name_index;
        obj_prop_addr->type = strtotype(type);
        kalos_object_property* obj_prop = append_list_item(builder, &current_export(builder)->entry.object.prop_list, sizeof(kalos_object_property));
        obj_prop->name_index = name_index;
        property = &obj_prop->property;
    } else {
        new_export(builder);
        current_export(builder)->name_index = strpack(builder, name);
        current_export(builder)->type = KALOS_EXPORT_TYPE_PROPERTY;
        property = &current_export(builder)->entry.property;
    }
    property->type = strtotype(type);
    if (strcmp(mode, "read") == 0) {
        property->read_invoke_id = builder->in_object ? 1 : ++builder->function_index;
        property->read_symbol_index = strpack(builder, symbol);
    } else if (strcmp(mode, "write") == 0) {
        property->write_invoke_id = builder->in_object ? 1 : ++builder->function_index;
        property->write_symbol_index = strpack(builder, symbol);
    } else if (strcmp(mode, "read,write") == 0) {
        property->read_invoke_id = builder->in_object ? 1 : ++builder->function_index;
        property->write_invoke_id = builder->in_object ? 1 : ++builder->function_index;
        property->read_symbol_index = strpack(builder, symbol);
        property->write_symbol_index = strpack(builder, symbol2);
    }
    LOG("prop %s %s %s %s %s", name, type, mode, symbol, symbol2);
}

kalos_module_parsed kalos_idl_parse_module(const char* s, kalos_basic_environment* env) {
    kalos_idl_callbacks callbacks = {
        error,
        prefix,
        dispatch_name,
        begin_module,
        end_module,
        begin_object,
        end_object,
        begin_function,
        function_arg,
        end_function,
        end_function_c,
        begin_handle,
        handle_arg,
        end_handle,
        constant_string,
        constant_number,
        property
    };

    struct kalos_module_builder context = {0};
    context.env = env;
    context.module_buffer_size = 1024;
    context.kalos_module_buffer = env->alloc(context.module_buffer_size);
    context.module_buffer_index = sizeof(kalos_module_header);
    memset(root(&context), 0, sizeof(kalos_module_header));
    strpack(&context, ""); // zero string = empty

    if (!kalos_idl_parse_callback(s, &context, &callbacks)) {
        kalos_module_parsed parsed = {0};
        return parsed;
    }

    kalos_int prop_addr_offset = root(&context)->prop_list.head;
    int index = 0;
    while (prop_addr_offset) {
        kalos_property_address* prop_addr = get_list_item(&context, prop_addr_offset);
        prop_addr->invoke_id = ++index;
        prop_addr_offset = prop_addr->prop_list.next;
        LOG("PROP %s=%d", context.string_buffer + prop_addr->name_index, prop_addr->invoke_id);
    }

    kalos_module_parsed parsed = kalos_buffer_alloc(env, context.module_buffer_index + context.string_buffer_index);
    memcpy(parsed.buffer, context.kalos_module_buffer, context.module_buffer_index);
    env->free(context.kalos_module_buffer);
    kalos_module_header* header = (kalos_module_header*)parsed.buffer;
    header->version = 1;
    header->string_offset = context.module_buffer_index;
    header->string_size = context.string_buffer_index;
    memcpy(PTR_BYTE_OFFSET(parsed.buffer, header->string_offset), context.string_buffer, context.string_buffer_index);
    env->free(context.string_buffer);

    return parsed;
}

static kalos_basic_environment* script_environment;
static kalos_module_parsed script_modules;
static kalos_module_header* script_current_header;
static kalos_module* script_current_module;
static kalos_export* script_current_export;
static kalos_function* script_current_function;
static kalos_object_property* script_current_property;

void kalos_idl_compiler_print(kalos_state state, kalos_string* string) {
    script_environment->print(kalos_string_c(state, *string));
}

void kalos_idl_compiler_println(kalos_state state, kalos_string* string) {
    script_environment->print(kalos_string_c(state, *string));
    script_environment->print("\n");
}

void kalos_idl_compiler_log(kalos_state state, kalos_string* string) {
    LOG("%s", kalos_string_c(state, *string));
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
        case FUNCTION_TYPE_VOID:
            return "void";
    }
    return "";
}

static void iter_function_arg(kalos_state state, void* context, uint16_t index, kalos_value* value) {
    kalos_int* record = context;
    kalos_arg* arg = kalos_module_get_list_item(script_modules, *record);
    *record = arg->arg_list.next;
    value->type = KALOS_VALUE_STRING;
    value->value.string = kalos_string_allocate(state, function_type_to_string(arg->type));
}

static kalos_string kalos_idl_module_name(kalos_state state) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_module->name_index)); }
static kalos_string kalos_idl_module_prefix(kalos_state state) { return kalos_string_allocate(state, script_current_header->prefix_index ? kalos_module_get_string(script_modules, script_current_header->prefix_index) : "kalos_idl_"); }
static kalos_string kalos_idl_obj_module_name(kalos_state state, kalos_object_ref* object) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_module->name_index)); }
static kalos_string kalos_idl_obj_module_prefix(kalos_state state, kalos_object_ref* object) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_header->prefix_index)); }

static kalos_string kalos_idl_export_name2(kalos_state state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_export->name_index)); }
static kalos_int kalos_idl_function_id2(kalos_state state, kalos_object_ref* o) { return script_current_function->invoke_id; }
static kalos_string kalos_idl_function_return_type2(kalos_state state, kalos_object_ref* o) { return kalos_string_allocate(state, function_type_to_string(script_current_function->return_type)); }
static kalos_string kalos_idl_function_realname2(kalos_state state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_function->symbol_index)); }
static kalos_string kalos_idl_function_c(kalos_state state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_function->c_index)); }
static kalos_object_ref kalos_idl_function_args2(kalos_state state, kalos_object_ref* o) {
    kalos_int* index;
    kalos_object_ref obj = kalos_allocate_sized_iterable(state, iter_function_arg, sizeof(kalos_int), (void**)&index, script_current_function->arg_list.count);
    *index = script_current_function->arg_list.head;
    return obj;
}
static kalos_int kalos_idl_function_arg_count2(kalos_state state, kalos_object_ref* o) { return script_current_function->arg_list.count; }
static kalos_string kalos_idl_function_varargs2(kalos_state state, kalos_object_ref* o) { return kalos_string_allocate(state, function_type_to_string(script_current_function->vararg_type)); }

static kalos_property* prop() { return script_current_property ? &script_current_property->property : &script_current_export->entry.property; }

static kalos_string kalos_idl_property_name(kalos_state state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_property ? script_current_property->name_index : script_current_export->name_index)); }
static kalos_int kalos_idl_property_read_id2(kalos_state state, kalos_object_ref* o) { return prop()->read_invoke_id; }
static kalos_int kalos_idl_property_write_id2(kalos_state state, kalos_object_ref* o) { return prop()->write_invoke_id; }
static kalos_string kalos_idl_property_read_symbol2(kalos_state state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, prop()->read_symbol_index)); }
static kalos_string kalos_idl_property_write_symbol2(kalos_state state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, prop()->write_symbol_index)); }
static kalos_string kalos_idl_property_type2(kalos_state state, kalos_object_ref* o) { return kalos_string_allocate(state, function_type_to_string(prop()->type)); }

static kalos_object_ref kalos_idl_handle_args(kalos_state state, kalos_object_ref* o) {
    kalos_int* index;
    kalos_object_ref obj = kalos_allocate_sized_iterable(state, iter_function_arg, sizeof(kalos_int), (void**)&index, script_current_export->entry.handler.arg_list.count);
    *index = script_current_export->entry.handler.arg_list.head;
    return obj;
}
static kalos_int kalos_idl_handle_index2(kalos_state state, kalos_object_ref* o) { return script_current_export->entry.handler.invoke_id; }
static kalos_int kalos_idl_handle_module_index2(kalos_state state, kalos_object_ref* o) { return script_current_module->index; }

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

void kalos_module_idl_module_trigger_property(kalos_state state, kalos_value* a0, kalos_object_ref* a1);

kalos_object_dispatch kalos_module_idl_module_object_property_obj_props;

void kalos_idl_walk_object_properties(kalos_state state, kalos_value* script_context, kalos_object_ref* object) {
    kalos_value v = kalos_value_clone(state, script_context);
    kalos_int prop_index = script_current_export->entry.object.prop_list.head;
    while (prop_index) {
        kalos_value ctx = kalos_value_clone(state, &v);
        script_current_property = kalos_module_get_list_item(script_modules, prop_index);
        // TODO: This is a bit of a hack
        if (script_current_property->property.read_invoke_id)
            script_current_property->property.read_invoke_id = kalos_module_lookup_property(script_modules, false, kalos_module_get_string(script_modules, script_current_property->name_index));
        if (script_current_property->property.write_invoke_id)
            script_current_property->property.write_invoke_id = kalos_module_lookup_property(script_modules, true, kalos_module_get_string(script_modules, script_current_property->name_index));
        kalos_object_ref obj = kalos_allocate_prop_object(state, NULL, &kalos_module_idl_module_object_property_obj_props);
        kalos_module_idl_module_trigger_property(state, &ctx, &obj);
        prop_index = script_current_property->prop_list.next;
    }
    kalos_clear(state, &v);
    script_current_property = NULL;
}

#include "kalos_idl_compiler.dispatch.inc"

bool export_walk_callback(void* context_, kalos_module_parsed parsed, uint16_t index, kalos_module* module, kalos_export* export) {
    struct walk_callback_context* context = context_;
    script_current_export = export;
    kalos_value ctx = kalos_value_clone(context->state, &context->script_context);
    kalos_object_ref obj = NULL;
    switch (export->type) {
        case KALOS_EXPORT_TYPE_FUNCTION:
            obj = kalos_allocate_prop_object(context->state, NULL, &kalos_module_idl_module_object_function_obj_props);
            script_current_function = kalos_module_get_list_item(parsed, export->entry.function_overload_list.head);
            kalos_module_idl_module_trigger_function(context->state, &ctx, &obj);
            script_current_function = NULL;
            break;
        case KALOS_EXPORT_TYPE_PROPERTY:
            obj = kalos_allocate_prop_object(context->state, NULL, &kalos_module_idl_module_object_property_obj_props);
            kalos_module_idl_module_trigger_property(context->state, &ctx, &obj);
            break;
        case KALOS_EXPORT_TYPE_HANDLE:
            obj = kalos_allocate_prop_object(context->state, NULL, &kalos_module_idl_module_object_handle_obj_props);
            kalos_module_idl_module_trigger_handle_(context->state, &ctx, &obj);
            break;
        case KALOS_EXPORT_TYPE_OBJECT:
            obj = kalos_allocate_prop_object(context->state, NULL, &kalos_module_idl_module_object_object_obj_props);
            kalos_module_idl_module_trigger_object(context->state, &ctx, &obj);
            break;
        default:
            break;
    }
    return true;
}

bool module_walk_callback(void* context_, kalos_module_parsed parsed, uint16_t index, kalos_module* module) {
    struct walk_callback_context* context = context_;
    script_current_module = module;
    kalos_value ctx = kalos_value_clone(context->state, &context->script_context);
    kalos_object_ref obj = kalos_allocate_prop_object(context->state, module, &kalos_module_idl_module_object_module_obj_props);
    kalos_module_idl_module_trigger_begin(context->state, &ctx, &obj);
    return true;
}

bool kalos_idl_generate_dispatch(kalos_module_parsed parsed_module, kalos_basic_environment* env) {
    // TODO: Watcom takes forever to compile if these aren't static
    static const char IDL_COMPILER_SCRIPT[] = {
        #include "kalos_idl_compiler.kalos.inc"
    };

    static const char IDL_COMPILER_IDL[] = {
        #include "kalos_idl_compiler.kidl.inc"
    };
    script_environment = env;
    kalos_module_parsed modules = kalos_idl_parse_module(IDL_COMPILER_IDL, env);
    if (!modules.buffer) {
        printf("ERROR: %s\n", "failed to parse compiler IDL");
        return false;
    }
    kalos_parse_options options = {0};
    kalos_script script = kalos_buffer_alloc(env, 8192);
    kalos_parse_result result = kalos_parse(IDL_COMPILER_SCRIPT, modules, options, &script);
    if (result.error) {
        printf("ERROR: %s\n", result.error);
        return false;
    }
    script_modules = parsed_module;
    script_current_header = (kalos_module_header*)parsed_module.buffer;
    kalos_dispatch dispatch = {0};
    dispatch.dispatch_name = kalos_module_idl_dynamic_dispatch;
    kalos_state state = kalos_init(&script, &dispatch, env);
    kalos_module_idl_trigger_open(state, (kalos_module_get_header(parsed_module)->flags & KALOS_MODULE_FLAG_DISPATCH_NAME) != 0);
    kalos_module_idl_trigger_close(state);
    kalos_run_free(state);
    return true;
}
