#pragma once

#include "kalos.h"
#include "_kalos_value.h"

typedef kalos_buffer kalos_module_parsed;

#define MAX_KALOS_EXPORT_NAME_LENGTH 64

typedef enum kalos_function_type {
    FUNCTION_TYPE_VOID,
    FUNCTION_TYPE_BOOL,
    FUNCTION_TYPE_NUMBER,
    FUNCTION_TYPE_OBJECT,
    FUNCTION_TYPE_STRING,
    FUNCTION_TYPE_ANY,
} kalos_function_type;

typedef enum kalos_export_type {
    KALOS_EXPORT_TYPE_CONST_NUMBER,
    KALOS_EXPORT_TYPE_CONST_STRING,
    KALOS_EXPORT_TYPE_FUNCTION,
    KALOS_EXPORT_TYPE_PROPERTY,
    KALOS_EXPORT_TYPE_HANDLER,
    KALOS_EXPORT_TYPE_OBJECT,
} kalos_export_type;

#pragma pack(push, 1)
typedef struct kalos_module_item_list {
    kalos_int prev, next;
} kalos_module_item_list;

typedef struct kalos_module_item_list_header {
    kalos_int head, tail, count;
} kalos_module_item_list_header;

typedef struct kalos_arg {
    kalos_module_item_list arg_list;
    kalos_int name_index;
    kalos_function_type type;
} kalos_arg;

typedef struct kalos_binding {
    kalos_int invoke_id;
    kalos_int symbol_index;
    kalos_int c_index;
} kalos_binding;

typedef struct kalos_function {
    kalos_module_item_list fn_overload_list;
    kalos_module_item_list_header arg_list;
    kalos_function_type vararg_type;
    kalos_function_type return_type;
    kalos_binding binding;
} kalos_function;

typedef struct kalos_handler {
    kalos_module_item_list_header arg_list;
    kalos_function_type vararg_type;
    kalos_function_type return_type;
    kalos_int invoke_id;
} kalos_handler;

typedef struct kalos_property {
    kalos_function_type type;
    kalos_binding read;
    kalos_binding write;
} kalos_property;

typedef struct kalos_object_property {
    kalos_module_item_list prop_list;
    kalos_property property;
    kalos_int name_index;
} kalos_object_property;

typedef struct kalos_object_def {
    kalos_module_item_list_header prop_list;
} kalos_object_def;

typedef union kalos_export_entry {
    kalos_module_item_list export_list;
    kalos_int const_number;
    kalos_int const_string_index;
    kalos_module_item_list_header function_overload_list;
    kalos_handler handler;
    kalos_property property;
    kalos_object_def object;
} kalos_export_entry;

typedef struct kalos_export {
    kalos_module_item_list export_list;
    kalos_int name_index;
    kalos_export_type type;
    kalos_export_entry entry;
} kalos_export;

typedef struct kalos_module {
    kalos_module_item_list module_list;
    kalos_int index;
    kalos_int name_index;
    kalos_module_item_list_header export_list;
} kalos_module;

#define KALOS_MODULE_FLAG_DISPATCH_NAME 1
#define KALOS_MODULE_FLAG_DISPATCH_INTERNAL 2

typedef struct kalos_module_header {
    kalos_int version;
    kalos_module_item_list_header module_list;
    kalos_module_item_list_header prop_list;
    kalos_int string_offset;
    kalos_int string_size;
    kalos_int prefix_index;
    kalos_int flags;
} kalos_module_header;

typedef struct kalos_export_address {
    kalos_int module_index;
    kalos_int export_index;
} kalos_export_address;

typedef struct kalos_property_address {
    kalos_module_item_list prop_list;
    kalos_function_type type;
    kalos_int invoke_id;
    kalos_int name_index;
} kalos_property_address;
#pragma pack(pop)

KALOS_MAYBE_UNUSED_BEGIN
static const kalos_export_address KALOS_GLOBAL_HANDLER_ADDRESS = { .module_index = -1, .export_index = -1 };
static const kalos_export_address KALOS_IDL_HANDLER_ADDRESS = { .module_index = -1, .export_index = -2 };
KALOS_MAYBE_UNUSED_END

static inline kalos_export_address kalos_make_address(kalos_int module_index, kalos_int export_index) {
    struct kalos_export_address res = { module_index, export_index };
    return res;
}

typedef bool (*kalos_module_callback)(void* context, kalos_module_parsed parsed, uint16_t index, kalos_module* module);
typedef bool (*kalos_export_callback)(void* context, kalos_module_parsed parsed, uint16_t index, kalos_module* module, kalos_export* export);

kalos_module* kalos_module_get_module(kalos_module_parsed parsed, kalos_int module_id);
kalos_module* kalos_module_find_module(kalos_module_parsed parsed, const char* name);
kalos_export* kalos_module_find_export(kalos_module_parsed parsed, kalos_module* module, const char* name);
void kalos_module_walk_modules(void* context, kalos_module_parsed parsed, kalos_module_callback callback);
void kalos_module_walk_exports(void* context, kalos_module_parsed parsed, kalos_module* module, kalos_export_callback callback);
kalos_int kalos_module_lookup_property(kalos_module_parsed parsed, bool write, const char* name);
const char* kalos_module_get_string(kalos_module_parsed parsed, kalos_int index);
void* kalos_module_get_list_item(kalos_module_parsed parsed, kalos_int offset);
kalos_module_header* kalos_module_get_header(kalos_module_parsed parsed);

#define BUILDER_HANDLE(x) typedef struct { kalos_int x##_index; } kalos_module_##x
typedef void* kalos_module_builder;
typedef kalos_module_item_list_header kalos_module_list;
typedef kalos_property kalos_module_property;
typedef kalos_binding kalos_module_binding;
BUILDER_HANDLE(arg);
BUILDER_HANDLE(export);
BUILDER_HANDLE(function);
BUILDER_HANDLE(module);
BUILDER_HANDLE(string);
BUILDER_HANDLE(object_property);
BUILDER_HANDLE(property_address);

kalos_module_builder kalos_module_create_builder(kalos_state* state, uint8_t* buffer, size_t size);
void kalos_module_free_builder(kalos_state* state, kalos_module_builder builder);

void kalos_module_create_idl(kalos_module_builder, kalos_module_string prefix, kalos_int flags, kalos_module_list modules, kalos_module_list prop_list);
kalos_module_module kalos_module_create_module(kalos_module_builder, kalos_int index, kalos_module_string name, kalos_module_list exports);
kalos_module_string kalos_module_create_string(kalos_module_builder, const char* s);
kalos_module_list kalos_module_create_list(kalos_module_builder);
void kalos_module_append_to_list(kalos_module_builder, kalos_module_list* list, void* handle);
kalos_module_export kalos_module_create_function_export(kalos_module_builder, kalos_module_string name, kalos_module_list overrides);
kalos_module_export kalos_module_create_property_export(kalos_module_builder, kalos_module_string name, kalos_module_property property);
kalos_module_export kalos_module_create_const_number_export(kalos_module_builder, kalos_module_string name, kalos_int value);
kalos_module_export kalos_module_create_const_string_export(kalos_module_builder, kalos_module_string name, kalos_module_string value);
kalos_module_export kalos_module_create_handler_export(kalos_module_builder, kalos_module_string name, kalos_int invoke_id, kalos_module_list args);
kalos_module_export kalos_module_create_object_export(kalos_module_builder, kalos_module_string name, kalos_module_list props);
kalos_module_function kalos_module_create_function(kalos_module_builder, kalos_function_type type, kalos_function_type varargs, kalos_module_list args, kalos_module_binding binding);
kalos_module_object_property kalos_module_create_object_property(kalos_module_builder, kalos_module_string name, kalos_module_property property);
void kalos_module_add_property_address(kalos_module_builder, kalos_module_list* list, kalos_module_string name);
kalos_module_property kalos_module_create_property(kalos_module_builder, kalos_function_type type, kalos_module_binding read, kalos_module_binding write);
kalos_module_binding kalos_module_create_binding_c(kalos_module_builder, kalos_int invoke_id, kalos_module_string c);
kalos_module_binding kalos_module_create_binding_function(kalos_module_builder, kalos_int invoke_id, kalos_module_string name);
kalos_module_arg kalos_module_create_arg(kalos_module_builder, kalos_module_string name, kalos_function_type type);
