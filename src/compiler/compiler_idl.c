#include "../_kalos_defines.h"
#include "../kalos_parse.h"
#include "../kalos_run.h"
#include "../_kalos_lex.h"
// #include "_kalos_idl_compiler.h"
#include "../modules/kalos_module_file.h"
#include "../modules/kalos_module_sys.h"
#include "compiler_gen.h"
#include "compiler_idl.h"

#define KALOS_IDL_HEADER_VERSION 1

#define ITERATE(name, list, sublist) {\
    ASSERT((list).type == KALOS_VALUE_OBJECT); \
    kalos_object_ref name##_obj = (list).value.object; \
    int length = name##_obj->getlength(state, &name##_obj); \
    for (int i = 0; i < length; i++) { \
        kalos_value sublist##_value = IDL_LIST_VALUE(name##_obj, i); \
        ASSERT(sublist##_value.type == KALOS_VALUE_OBJECT); \
        kalos_object_ref sublist = sublist##_value.value.object;
#define ITERATE_END }}
#define ITERATE_TYPE(name, list, sublist, type_) \
    ITERATE(name, list, sublist) \
        kalos_value name##_entry_value = IDL_LIST_VALUE(sublist, 0); \
        ASSERT(name##_entry_value.type == KALOS_VALUE_NUMBER); \
        kalos_token type_ = name##_entry_value.value.number; \
        switch (type_) { default: kalos_value_error(kalos_state_from_run_state(run_state)); return; 
#define ITERATE_TYPE_END ITERATE_END }
#define ITERATE_CASE(token) break; case token
#define IDL_LIST_STRING(sublist, index) kalos_module_create_string(builder, kalos_string_c(state, IDL_LIST_VALUE(sublist, index).value.string))
#define IDL_LIST_NUMBER(sublist, index) IDL_LIST_VALUE(sublist, index).value.number
#define IDL_LIST_OBJECT(sublist, index) IDL_LIST_VALUE(sublist, index).value.object
#define IDL_LIST_VALUE(sublist, index) release(state, sublist->getindex(state, &sublist, (index)))

/**
 * Eagerly release the reference as we don't need to keep this around and we know the list
 * has it ref'd.
 */
static inline kalos_value release(kalos_state* state, kalos_value v) {
    kalos_value vcopy = v;
    kalos_clear(state, &v);
    return vcopy;
}

kalos_binding kalos_idl_binding(kalos_module_builder builder, kalos_state* state, kalos_object_ref binding_list) {
    if (binding_list->getlength(state, &binding_list) == 0) {
        kalos_binding binding = {0};
        return binding;
    }
    kalos_token binding_type = IDL_LIST_NUMBER(binding_list, 0);
    kalos_module_string binding_string_index = IDL_LIST_STRING(binding_list, 1);
    kalos_int invoke_id = IDL_LIST_NUMBER(binding_list, 2);
    return binding_type == KALOS_TOKEN_C ? kalos_module_create_binding_c(builder, invoke_id, binding_string_index) : kalos_module_create_binding_function(builder, invoke_id, binding_string_index);
}

kalos_property kalos_idl_property(kalos_module_builder builder, kalos_state* state, kalos_object_ref prop_list) {
    kalos_binding read_binding = kalos_idl_binding(builder, state, IDL_LIST_OBJECT(prop_list, 3));
    kalos_binding write_binding = kalos_idl_binding(builder, state, IDL_LIST_OBJECT(prop_list, 4));
    return kalos_module_create_property(builder, IDL_LIST_NUMBER(prop_list, 2), read_binding, write_binding);
}

void kalos_idl_callback(kalos_run_state* run_state, kalos_value* idl) {
    kalos_state* state = kalos_state_from_run_state(run_state);
    kalos_module_builder builder = kalos_module_create_builder(state);
    kalos_module_string prefix_index = kalos_module_create_string(builder, "");
    kalos_int flags = 0;
    kalos_module_list modules = kalos_module_create_list(builder);
    kalos_module_list prop_list = kalos_module_create_list(builder);
    ITERATE_TYPE(idl, *idl, sublist, type) {
        ITERATE_CASE(KALOS_TOKEN_PREFIX): {
            prefix_index = IDL_LIST_STRING(sublist, 1);
        }
        ITERATE_CASE(KALOS_TOKEN_DISPATCH): {
            if (IDL_LIST_NUMBER(sublist, 1) == KALOS_TOKEN_NAME) {
                flags = KALOS_MODULE_FLAG_DISPATCH_NAME;
            } else if (IDL_LIST_NUMBER(sublist, 1) == KALOS_TOKEN_INTERNAL) {
                flags = KALOS_MODULE_FLAG_DISPATCH_INTERNAL;
            }
        }
        ITERATE_CASE(KALOS_TOKEN_MODULE): {
            kalos_module_list exports = kalos_module_create_list(builder);
            ITERATE_TYPE(module, IDL_LIST_VALUE(sublist, 3), export, type) {
                ITERATE_CASE(KALOS_TOKEN_FN): {
                    kalos_module_string fn_name_index = IDL_LIST_STRING(export, 1);
                    kalos_module_list overrides = kalos_module_create_list(builder);
                    ITERATE(fn, IDL_LIST_VALUE(export, 2), overload) {
                        kalos_module_list args = kalos_module_create_list(builder);
                        kalos_function_type varargs_type = FUNCTION_TYPE_VOID;
                        ITERATE(overload, IDL_LIST_VALUE(overload, 0), arglist) {
                            if (arglist->getlength(state, &arglist) == 2) { 
                                kalos_module_string arg_name_index = IDL_LIST_STRING(arglist, 0);
                                kalos_module_arg arg = kalos_module_create_arg(builder, arg_name_index, IDL_LIST_NUMBER(arglist, 1));
                                kalos_module_append_to_list(builder, &args, &arg);
                            } else {
                                varargs_type = IDL_LIST_NUMBER(arglist, 1);
                            }
                        } ITERATE_END;
                        kalos_module_binding binding = kalos_idl_binding(builder, state, IDL_LIST_OBJECT(overload, 2));
                        kalos_module_function fn = kalos_module_create_function(builder, IDL_LIST_NUMBER(overload, 1), varargs_type, args, binding);
                        kalos_module_append_to_list(builder, &overrides, &fn);
                    } ITERATE_END;
                    kalos_module_export fn_export = kalos_module_create_function_export(builder, fn_name_index, overrides);
                    kalos_module_append_to_list(builder, &exports, &fn_export);
                }
                ITERATE_CASE(KALOS_TOKEN_PROP): {
                    kalos_module_string prop_name_index = IDL_LIST_STRING(export, 1);
                    kalos_property property = kalos_idl_property(builder, state, export);
                    kalos_module_export prop_export = kalos_module_create_property_export(builder, prop_name_index, property);
                    kalos_module_append_to_list(builder, &exports, &prop_export);
                }
                ITERATE_CASE(KALOS_TOKEN_OBJECT): {
                    kalos_module_string obj_name_index = IDL_LIST_STRING(export, 1);
                    kalos_module_list props = kalos_module_create_list(builder);
                    ITERATE(prop, IDL_LIST_VALUE(export, 2), prop) {
                        kalos_module_string prop_name_index = IDL_LIST_STRING(prop, 1);
                        kalos_module_object_property property = kalos_module_create_object_property(builder, prop_name_index, kalos_idl_property(builder, state, prop));
                        kalos_module_append_to_list(builder, &props, &property);
                        kalos_module_add_property_address(builder, &prop_list, prop_name_index);
                    } ITERATE_END;
                    kalos_module_export obj_export = kalos_module_create_object_export(builder, obj_name_index, props);
                    kalos_module_append_to_list(builder, &exports, &obj_export);
                }
                ITERATE_CASE(KALOS_TOKEN_HANDLER): {
                    kalos_module_string handler_name_index = IDL_LIST_STRING(export, 1);
                    kalos_module_list args = kalos_module_create_list(builder);
                    ITERATE(arg, IDL_LIST_VALUE(export, 3), arglist) {
                        kalos_module_string arg_name_index = IDL_LIST_STRING(arglist, 0);
                        kalos_module_arg arg = kalos_module_create_arg(builder, arg_name_index, IDL_LIST_NUMBER(arglist, 1));
                        kalos_module_append_to_list(builder, &args, &arg);
                    } ITERATE_END;
                    kalos_module_export handler_export = kalos_module_create_handler_export(builder, handler_name_index, IDL_LIST_NUMBER(export, 2), args);
                    kalos_module_append_to_list(builder, &exports, &handler_export);
                }
                ITERATE_CASE(KALOS_TOKEN_CONST): {
                    kalos_module_string const_name_index = IDL_LIST_STRING(export, 1);
                    kalos_value value = IDL_LIST_VALUE(export, 3);
                    kalos_module_export const_export = value.type == KALOS_VALUE_STRING
                        ? kalos_module_create_const_string_export(builder, const_name_index, IDL_LIST_STRING(export, 3))
                        : kalos_module_create_const_number_export(builder, const_name_index, IDL_LIST_NUMBER(export, 3));
                    kalos_module_append_to_list(builder, &exports, &const_export);
                }
            } ITERATE_TYPE_END;
            kalos_module_string module_name_index = IDL_LIST_STRING(sublist, 2);
            kalos_module_module module = kalos_module_create_module(builder, IDL_LIST_NUMBER(sublist, 1), module_name_index, exports);
            kalos_module_append_to_list(builder, &modules, &module);
        }
    } ITERATE_TYPE_END;

    kalos_module_create_idl(builder, prefix_index, flags, modules, prop_list);
    *((kalos_buffer*)state->context) = kalos_module_finish_builder(state, builder);
}

kalos_module_parsed kalos_idl_parse_module(const char* s, kalos_state* state) {
    kalos_parse_options options = {0};
    kalos_buffer idl;
    kalos_module_parsed parsed = {0};
    kalos_parse_result result = kalos_parse_buffer(s, parsed, options, state, &idl);
    if (result.error) {
        if (state->error) {
            state->error(state->context, result.line, result.error);
        }
        return parsed;
    }
    kalos_dispatch dispatch;
    dispatch.idl = kalos_idl_callback;
    kalos_state idl_parser_state = *state;
    kalos_buffer buffer = {0};
    idl_parser_state.context = &buffer;
    kalos_run_state* run_state = kalos_init((const_kalos_script)idl.buffer, &dispatch, &idl_parser_state);
    kalos_trigger(run_state, KALOS_IDL_HANDLER_ADDRESS);
    kalos_run_free(run_state);
    kalos_buffer_free(idl);
    return buffer;
}

static kalos_module_parsed script_modules;
static kalos_module_header* script_current_header;
static kalos_module* script_current_module;
static kalos_export* script_current_export;
static kalos_object_property* script_current_property;

void kalos_idl_compiler_print(kalos_state* state, kalos_string* string) {
    state->print(state->context, kalos_string_c(state, *string));
}

void kalos_idl_compiler_println(kalos_state* state, kalos_string* string) {
    state->print(state->context, kalos_string_c(state, *string));
    state->print(state->context, "\n");
}

void kalos_idl_compiler_log(kalos_state* state, kalos_string* string) {
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

kalos_object_dispatch kalos_module_idl_module_object_property_obj_props;
kalos_object_dispatch kalos_module_idl_module_object_overload_obj_props;
kalos_object_dispatch kalos_module_idl_module_object_binding_obj_props;
kalos_object_dispatch kalos_module_idl_module_object_module_obj_props;
kalos_object_dispatch kalos_module_idl_kalos_object_idl_props;

struct iter_context {
    kalos_module_parsed modules;
    kalos_object_dispatch* object;
    kalos_int index;
};

static void iter_list_object(kalos_state* state, void* context_, uint16_t index, kalos_value* value) {
    struct iter_context* context = context_;
    kalos_module_item_list* list = kalos_module_get_list_item(context->modules, context->index);
    context->index = list->next;
    value->type = KALOS_VALUE_OBJECT;
    value->value.object = kalos_allocate_prop_object(state, list, context->object);
}

static kalos_object_ref make_list_iterator(kalos_state* state, kalos_module_parsed modules, kalos_module_item_list_header* list, kalos_object_dispatch* object) {
    struct iter_context* context;
    kalos_object_ref obj = kalos_allocate_sized_iterable(state, false, iter_list_object, sizeof(struct iter_context), (void**)&context, list->count);
    context->modules = modules;
    context->index = list->head;
    context->object = object;
    return obj;
}

kalos_object_ref kalos_idl_get_modules(kalos_state* state, kalos_object_ref* object) {
    ASSERT((*object)->dispatch == &kalos_module_idl_kalos_object_idl_props);
    kalos_module_parsed* modules = (*object)->context;
    kalos_module_header* header = kalos_module_get_header(*modules);
    // ASSERT(header->version == KALOS_IDL_HEADER_VERSION);
    return make_list_iterator(state, *modules, &header->module_list, &kalos_module_idl_module_object_module_obj_props);
}

static void iter_function_arg(kalos_state* state, void* context, uint16_t index, kalos_value* value) {
    kalos_int* record = context;
    kalos_arg* arg = kalos_module_get_list_item(script_modules, *record);
    *record = arg->arg_list.next;
    value->type = KALOS_VALUE_STRING;
    value->value.string = kalos_string_allocate(state, function_type_to_string(arg->type));
}

static void iter_function_overload(kalos_state* state, void* context, uint16_t index, kalos_value* value) {
    kalos_int* record = context;
    kalos_function* fn = kalos_module_get_list_item(script_modules, *record);
    *record = fn->fn_overload_list.next;
    value->type = KALOS_VALUE_OBJECT;
    value->value.object = kalos_allocate_prop_object(state, fn, &kalos_module_idl_module_object_overload_obj_props);
}

#define IDL_STRING(x) kalos_string_allocate((void*)state, kalos_module_get_string(script_modules, x))
#define IDL_OBJECT(type) ((type*)(*object)->context)

static kalos_string kalos_idl_module_prefix(kalos_state* state) { return kalos_string_allocate(state, script_current_header->prefix_index ? kalos_module_get_string(script_modules, script_current_header->prefix_index) : "kalos_idl_"); }

static kalos_string kalos_idl_export_name(kalos_state* state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_export->name_index)); }
static kalos_object_ref kalos_idl_function_overloads(kalos_state* state, kalos_object_ref* o) {
    kalos_int* index;
    kalos_object_ref obj = kalos_allocate_sized_iterable(state, false, iter_function_overload, sizeof(kalos_int), (void**)&index, script_current_export->entry.function_overload_list.count);
    *index = script_current_export->entry.function_overload_list.head;
    return obj;
}

static kalos_string kalos_idl_overload_return_type(kalos_state* state, kalos_object_ref* object) { return kalos_string_allocate(state, function_type_to_string(IDL_OBJECT(kalos_function)->return_type)); }
static kalos_object_ref kalos_idl_overload_args(kalos_state* state, kalos_object_ref* object) {
    kalos_int* index;
    kalos_object_ref obj = kalos_allocate_sized_iterable(state, false, iter_function_arg, sizeof(kalos_int), (void**)&index, IDL_OBJECT(kalos_function)->arg_list.count);
    *index = IDL_OBJECT(kalos_function)->arg_list.head;
    return obj;
}
static kalos_int kalos_idl_overload_arg_count(kalos_state* state, kalos_object_ref* object) { return IDL_OBJECT(kalos_function)->arg_list.count; }
static kalos_string kalos_idl_overload_varargs(kalos_state* state, kalos_object_ref* object) { return kalos_string_allocate(state, function_type_to_string(IDL_OBJECT(kalos_function)->vararg_type)); }
static kalos_object_ref kalos_idl_overload_binding(kalos_state* state, kalos_object_ref* object) { return kalos_allocate_prop_object(state, &IDL_OBJECT(kalos_function)->binding, &kalos_module_idl_module_object_binding_obj_props ); }

static kalos_property* prop() { return script_current_property ? &script_current_property->property : &script_current_export->entry.property; }
static kalos_string kalos_idl_property_name(kalos_state* state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_property ? script_current_property->name_index : script_current_export->name_index)); }
static kalos_string kalos_idl_property_type(kalos_state* state, kalos_object_ref* o) { return kalos_string_allocate(state, function_type_to_string(prop()->type)); }
static kalos_object_ref kalos_idl_property_read_binding(kalos_state* state, kalos_object_ref* o) { return kalos_allocate_prop_object(state, &prop()->read, &kalos_module_idl_module_object_binding_obj_props ); }
static kalos_object_ref kalos_idl_property_write_binding(kalos_state* state, kalos_object_ref* o) { return kalos_allocate_prop_object(state, &prop()->write, &kalos_module_idl_module_object_binding_obj_props ); }

static kalos_object_ref kalos_idl_handler_args(kalos_state* state, kalos_object_ref* o) {
    kalos_int* index;
    kalos_object_ref obj = kalos_allocate_sized_iterable(state, false, iter_function_arg, sizeof(kalos_int), (void**)&index, script_current_export->entry.handler.arg_list.count);
    *index = script_current_export->entry.handler.arg_list.head;
    return obj;
}
static kalos_int kalos_idl_handler_index(kalos_state* state, kalos_object_ref* o) { return script_current_export->entry.handler.invoke_id; }
static kalos_int kalos_idl_handler_module_index(kalos_state* state, kalos_object_ref* o) { return script_current_module->index; }

static kalos_string kalos_idl_binding_type(kalos_state* state, kalos_object_ref* o) { return ((kalos_binding*)(*o)->context)->c_index ? kalos_string_allocate(state, "c") : kalos_string_allocate(state, "fn"); }
static kalos_string kalos_idl_binding_value(kalos_state* state, kalos_object_ref* o) { return ((kalos_binding*)(*o)->context)->c_index ? IDL_STRING(((kalos_binding*)(*o)->context)->c_index) : IDL_STRING(((kalos_binding*)(*o)->context)->symbol_index); }

struct walk_callback_context {
    bool handlers;
    kalos_module_parsed modules;
    kalos_run_state* state;
    kalos_value script_context;
};

bool export_walk_callback(void* context_, kalos_module_parsed parsed, uint16_t index, kalos_module* module, kalos_export* export);
bool module_walk_callback(void* context_, kalos_module_parsed parsed, uint16_t index, kalos_module* module);

void kalos_idl_walk_modules(kalos_run_state* run_state, kalos_value* script_context) {
    kalos_state* state = kalos_state_from_run_state(run_state);
    struct walk_callback_context context;
    context.state = run_state;
    kalos_value_move_to(state, script_context, &context.script_context);
    context.modules = script_modules;
    kalos_module_walk_modules(&context, script_modules, module_walk_callback);
    kalos_clear(state, &context.script_context);
}

void kalos_idl_walk_exports(kalos_run_state* run_state, kalos_value* script_context) {
    kalos_state* state = kalos_state_from_run_state(run_state);
    struct walk_callback_context context;
    context.state = run_state;
    kalos_value_move_to(state, script_context, &context.script_context);
    context.modules = script_modules;
    kalos_module_walk_exports(&context, script_modules, script_current_module, export_walk_callback);
    kalos_clear(state, &context.script_context);
}

void kalos_module_idl_module_trigger_property(kalos_run_state* state, kalos_value* a0, kalos_object_ref* a1);

void kalos_idl_walk_object_properties(kalos_run_state* run_state, kalos_value* script_context, kalos_object_ref* object) {
    kalos_state* state = kalos_state_from_run_state(run_state);
    kalos_value v = kalos_value_clone(state, script_context);
    kalos_int prop_index = script_current_export->entry.object.prop_list.head;
    while (prop_index) {
        kalos_value ctx = kalos_value_clone(state, &v);
        script_current_property = kalos_module_get_list_item(script_modules, prop_index);
        // TODO: This is a bit of a hack
        if (script_current_property->property.read.invoke_id)
            script_current_property->property.read.invoke_id = kalos_module_lookup_property(script_modules, false, kalos_module_get_string(script_modules, script_current_property->name_index));
        if (script_current_property->property.write.invoke_id)
            script_current_property->property.write.invoke_id = kalos_module_lookup_property(script_modules, true, kalos_module_get_string(script_modules, script_current_property->name_index));
        kalos_object_ref obj = kalos_allocate_prop_object(state, NULL, &kalos_module_idl_module_object_property_obj_props);
        kalos_module_idl_module_trigger_property(run_state, &ctx, &obj);
        prop_index = script_current_property->prop_list.next;
    }
    kalos_clear(state, &v);
    script_current_property = NULL;
}

KALOS_ALLOW_UNREACHABLE_CODE_BEGIN
#include "compiler.dispatch.inc"
KALOS_ALLOW_UNREACHABLE_CODE_END

bool export_walk_callback(void* context_, kalos_module_parsed parsed, uint16_t index, kalos_module* module, kalos_export* export) {
    struct walk_callback_context* context = context_;
    kalos_state* state = kalos_state_from_run_state(context->state);
    script_current_export = export;
    kalos_value ctx = kalos_value_clone(state, &context->script_context);
    kalos_object_ref obj = NULL;
    switch (export->type) {
        case KALOS_EXPORT_TYPE_FUNCTION:
            obj = kalos_allocate_prop_object(state, NULL, &kalos_module_idl_module_object_function_obj_props);
            kalos_module_idl_module_trigger_function(context->state, &ctx, &obj);
            break;
        case KALOS_EXPORT_TYPE_PROPERTY:
            obj = kalos_allocate_prop_object(state, NULL, &kalos_module_idl_module_object_property_obj_props);
            kalos_module_idl_module_trigger_property(context->state, &ctx, &obj);
            break;
        case KALOS_EXPORT_TYPE_HANDLER:
            obj = kalos_allocate_prop_object(state, NULL, &kalos_module_idl_module_object_handler_obj_props);
            kalos_module_idl_module_trigger_handler(context->state, &ctx, &obj);
            break;
        case KALOS_EXPORT_TYPE_OBJECT:
            obj = kalos_allocate_prop_object(state, NULL, &kalos_module_idl_module_object_object_obj_props);
            kalos_module_idl_module_trigger_object(context->state, &ctx, &obj);
            break;
        default:
            break;
    }
    return true;
}

bool module_walk_callback(void* context_, kalos_module_parsed parsed, uint16_t index, kalos_module* module) {
    struct walk_callback_context* context = context_;
    kalos_state* state = kalos_state_from_run_state(context->state);
    script_current_module = module;
    kalos_value ctx = kalos_value_clone(state, &context->script_context);
    kalos_object_ref obj = kalos_allocate_prop_object(state, module, &kalos_module_idl_module_object_module_obj_props);
    kalos_module_idl_module_trigger_begin(context->state, &ctx, &obj);
    return true;
}

bool kalos_idl_generate_dispatch(kalos_module_parsed parsed_module, kalos_state* state) {
    kalos_buffer script = compiler_idl_script(state);
    script_modules = parsed_module;
    script_current_header = (kalos_module_header*)parsed_module.buffer;
    kalos_dispatch dispatch = {0};
    dispatch.dispatch_name = kalos_module_idl_dynamic_dispatch;
    kalos_run_state* run_state = kalos_init((const_kalos_script)script.buffer, &dispatch, state);
    kalos_module_idl_trigger_open(run_state, kalos_module_get_header(parsed_module)->flags);
    kalos_module_idl_trigger_close(run_state);
    kalos_run_free(run_state);
    kalos_buffer_free(script);
    return true;
}

kalos_string kalos_compiler_get_buffer(kalos_state* state, kalos_object_ref* object) {
    kalos_buffer* buffer = (*object)->context;
    size_t size = kalos_buffer_size(*buffer);
    kalos_writable_string s = kalos_string_allocate_writable_size(state, size);
    memcpy(kalos_string_writable_c(state, s), buffer->buffer, size);
    // memcpy(kalos_string_writable_c(state, s), "hello", 6);
    return kalos_string_commit_length(state, s, size);
}

void buffer_free(kalos_state* state, kalos_object_ref* object) {
    kalos_buffer* buffer = (*object)->context;
    kalos_buffer_free(*buffer);
}

kalos_object_ref kalos_compiler_compile_idl(kalos_state* state, kalos_string* idl) {
    kalos_module_parsed parsed = kalos_idl_parse_module(kalos_string_c(state, *idl), state);
    kalos_object_ref object = kalos_allocate_object(state, sizeof(parsed));
    memcpy(object->context, &parsed, sizeof(parsed));
    object->dispatch = &kalos_module_idl_kalos_object_idl_props;
    object->object_free = buffer_free;
    return object;
}

kalos_object_ref kalos_compiler_compile_script(kalos_state* state, kalos_object_ref* modules_, kalos_string* script) {
    kalos_parse_options options = {0};
    kalos_buffer buffer;
    kalos_module_parsed* modules = (*modules_)->context;
    kalos_parse_result result = kalos_parse_buffer(kalos_string_c(state, *script), *modules, options, state, &buffer);
    kalos_object_ref object = kalos_allocate_object(state, sizeof(buffer));
    memcpy(object->context, &buffer, sizeof(buffer));
    object->dispatch = &kalos_module_idl_kalos_object_script_props;
    object->object_free = buffer_free;
    return object;
}

void kalos_compiler_generate_idl(kalos_state* state, kalos_object_ref* modules_) {
    kalos_module_parsed* modules = (*modules_)->context;
    kalos_idl_generate_dispatch(*modules, state);
}

kalos_string kalos_compiler_get_idl_script(kalos_state* state) {
    return kalos_string_allocate(state, compiler_idl_script_text_inc());
}

kalos_string kalos_compiler_get_compiler_idl(kalos_state* state) {
    return kalos_string_allocate(state, compiler_kidl_text_inc());
}

void kalos_compiler_run_script(kalos_state* state, kalos_object_ref* script, kalos_object_ref* args) {
    kalos_dispatch dispatch = {0};
    dispatch.dispatch_name = kalos_module_idl_dynamic_dispatch;
    kalos_buffer* buffer = (*script)->context;
    kalos_run_state* run_state = kalos_init(buffer->buffer, &dispatch, state);
    kalos_module_idl_sys_trigger_main(run_state, args);
    kalos_run_free(run_state);
}
