#pragma once

#include "kalos.h"
#include "kalos_value.h"

#define MAX_KALOS_EXPORT_NAME_LENGTH 64
#define MAX_KALOS_ARGS 8
#define MAX_KALOS_OBJ_PROPS 8

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
    KALOS_EXPORT_TYPE_HANDLE,
    KALOS_EXPORT_TYPE_OBJECT,
} kalos_export_type;

typedef struct kalos_module_parsed {
    void* data;
    size_t size;
} kalos_module_parsed;

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

typedef struct kalos_function {
    kalos_module_item_list fn_overload_list;
    kalos_module_item_list_header arg_list;
    kalos_function_type vararg_type;
    kalos_function_type return_type;
    kalos_int symbol_index;
    kalos_int c_index;
    kalos_int invoke_id;
} kalos_function;

typedef struct kalos_handler {
    kalos_module_item_list_header arg_list;
    uint8_t arg_count;
    kalos_function_type vararg_type;
    kalos_function_type return_type;
    kalos_int invoke_id;
} kalos_handler;

typedef struct kalos_property {
    kalos_function_type type;
    kalos_int read_symbol_index;
    kalos_int read_invoke_id;
    kalos_int write_symbol_index;
    kalos_int write_invoke_id;
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

typedef bool (*kalos_dispatch_name_fn)(kalos_state state, const char* module, const char* name, int param_count, kalos_stack* stack, bool retval);
typedef void (*kalos_dispatch_fn)(kalos_state state, int function, int param_count, kalos_stack* stack, bool retval);

typedef struct kalos_dispatch {
    kalos_dispatch_name_fn dispatch_name;
    kalos_dispatch_fn* modules;
} kalos_dispatch;

typedef struct kalos_module {
    kalos_module_item_list module_list;
    kalos_int index;
    kalos_int name_index;
    kalos_int export_count;
    kalos_module_item_list_header export_list;
} kalos_module;

#define KALOS_MODULE_FLAG_DISPATCH_NAME 1

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

static const kalos_export_address KALOS_GLOBAL_HANDLE_ADDRESS = { .module_index = -1, .export_index = -1 };

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
