#include "_kalos_defines.h"
#include "kalos_parse.h"
#include "kalos_run.h"
#include "_kalos_idl_compiler.h"
#include "_kalos_lex.h"

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
    kalos_module_string prop_name_index = IDL_LIST_STRING(prop_list, 1);
    kalos_binding read_binding = kalos_idl_binding(builder, state, IDL_LIST_OBJECT(prop_list, 3));
    kalos_binding write_binding = kalos_idl_binding(builder, state, IDL_LIST_OBJECT(prop_list, 4));
    return kalos_module_create_property(builder, IDL_LIST_NUMBER(prop_list, 2), read_binding, write_binding);
}

void kalos_idl_callback(kalos_run_state* run_state, kalos_value* idl) {
    kalos_state* state = kalos_state_from_run_state(run_state);
    kalos_buffer* idl_buffer = (kalos_buffer*)run_state->context;
    kalos_module_builder builder = kalos_module_create_builder(state, idl_buffer->buffer, kalos_buffer_size(*idl_buffer));
    kalos_module_string prefix_index = kalos_module_create_string(builder, "");
    bool dispatch_name = false;
    kalos_module_list modules = kalos_module_create_list(builder);
    kalos_module_list prop_list = kalos_module_create_list(builder);
    ITERATE_TYPE(idl, *idl, sublist, type) {
        ITERATE_CASE(KALOS_TOKEN_PREFIX): {
            prefix_index = IDL_LIST_STRING(sublist, 1);
        }
        ITERATE_CASE(KALOS_TOKEN_DISPATCH): {
            dispatch_name = true;
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

    kalos_module_create_idl(builder, prefix_index, dispatch_name, modules, prop_list);
    kalos_module_free_builder(state, builder);
}

kalos_module_parsed kalos_idl_parse_module(const char* s, kalos_state* state) {
    kalos_parse_options options = {0};
    kalos_module_parsed parsed = {0};
    kalos_buffer idl = kalos_buffer_alloc(state, 10 * 1024);
    kalos_parse_result result = kalos_parse(s, parsed, options, idl.buffer);
    if (result.error) {
        kalos_buffer_free(idl);
        return parsed;
    }
    kalos_dispatch dispatch = {
        .idl = kalos_idl_callback
    };
    kalos_state idl_parser_state = *state;
    kalos_buffer idl_buffer = kalos_buffer_alloc(state, 10 * 1024);    
    idl_parser_state.context = &idl_buffer;
    kalos_run_state* run_state = kalos_init((const_kalos_script)idl.buffer, &dispatch, &idl_parser_state);
    kalos_trigger(run_state, KALOS_IDL_HANDLER_ADDRESS);
    kalos_run_free(run_state);
    kalos_buffer_free(idl);
    return idl_buffer;
}

static kalos_state* script_environment;
static kalos_module_parsed script_modules;
static kalos_module_header* script_current_header;
static kalos_module* script_current_module;
static kalos_export* script_current_export;
static kalos_function* script_current_function;
static kalos_object_property* script_current_property;

void kalos_idl_compiler_print(kalos_state* state, kalos_string* string) {
    script_environment->print(kalos_string_c(state, *string));
}

void kalos_idl_compiler_println(kalos_state* state, kalos_string* string) {
    script_environment->print(kalos_string_c(state, *string));
    script_environment->print("\n");
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

static void iter_function_arg(kalos_state* state, void* context, uint16_t index, kalos_value* value) {
    kalos_int* record = context;
    kalos_arg* arg = kalos_module_get_list_item(script_modules, *record);
    *record = arg->arg_list.next;
    value->type = KALOS_VALUE_STRING;
    value->value.string = kalos_string_allocate(state, function_type_to_string(arg->type));
}

static kalos_string kalos_idl_module_name(kalos_state* state) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_module->name_index)); }
static kalos_string kalos_idl_module_prefix(kalos_state* state) { return kalos_string_allocate(state, script_current_header->prefix_index ? kalos_module_get_string(script_modules, script_current_header->prefix_index) : "kalos_idl_"); }
static kalos_string kalos_idl_obj_module_name(kalos_state* state, kalos_object_ref* object) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_module->name_index)); }
static kalos_string kalos_idl_obj_module_prefix(kalos_state* state, kalos_object_ref* object) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_header->prefix_index)); }

static kalos_string kalos_idl_export_name2(kalos_state* state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_export->name_index)); }
static kalos_int kalos_idl_function_id2(kalos_state* state, kalos_object_ref* o) { return script_current_function->binding.invoke_id; }
static kalos_string kalos_idl_function_return_type2(kalos_state* state, kalos_object_ref* o) { return kalos_string_allocate(state, function_type_to_string(script_current_function->return_type)); }
static kalos_string kalos_idl_function_realname2(kalos_state* state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_function->binding.symbol_index)); }
static kalos_string kalos_idl_function_c(kalos_state* state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_function->binding.c_index)); }
static kalos_object_ref kalos_idl_function_args2(kalos_state* state, kalos_object_ref* o) {
    kalos_int* index;
    kalos_object_ref obj = kalos_allocate_sized_iterable(state, iter_function_arg, sizeof(kalos_int), (void**)&index, script_current_function->arg_list.count);
    *index = script_current_function->arg_list.head;
    return obj;
}
static kalos_int kalos_idl_function_arg_count2(kalos_state* state, kalos_object_ref* o) { return script_current_function->arg_list.count; }
static kalos_string kalos_idl_function_varargs2(kalos_state* state, kalos_object_ref* o) { return kalos_string_allocate(state, function_type_to_string(script_current_function->vararg_type)); }

static kalos_property* prop() { return script_current_property ? &script_current_property->property : &script_current_export->entry.property; }

static kalos_string kalos_idl_property_name(kalos_state* state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, script_current_property ? script_current_property->name_index : script_current_export->name_index)); }
static kalos_int kalos_idl_property_read_id2(kalos_state* state, kalos_object_ref* o) { return prop()->read.invoke_id; }
static kalos_int kalos_idl_property_write_id2(kalos_state* state, kalos_object_ref* o) { return prop()->write.invoke_id; }
static kalos_string kalos_idl_property_read_symbol2(kalos_state* state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, prop()->read.symbol_index)); }
static kalos_string kalos_idl_property_write_symbol2(kalos_state* state, kalos_object_ref* o) { return kalos_string_allocate(state, kalos_module_get_string(script_modules, prop()->write.symbol_index)); }
static kalos_string kalos_idl_property_type2(kalos_state* state, kalos_object_ref* o) { return kalos_string_allocate(state, function_type_to_string(prop()->type)); }

static kalos_object_ref kalos_idl_handler_args(kalos_state* state, kalos_object_ref* o) {
    kalos_int* index;
    kalos_object_ref obj = kalos_allocate_sized_iterable(state, iter_function_arg, sizeof(kalos_int), (void**)&index, script_current_export->entry.handler.arg_list.count);
    *index = script_current_export->entry.handler.arg_list.head;
    return obj;
}
static kalos_int kalos_idl_handler_index2(kalos_state* state, kalos_object_ref* o) { return script_current_export->entry.handler.invoke_id; }
static kalos_int kalos_idl_handler_module_index2(kalos_state* state, kalos_object_ref* o) { return script_current_module->index; }

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

kalos_object_dispatch kalos_module_idl_module_object_property_obj_props;

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

#include "_kalos_idl_compiler.dispatch.inc"

bool export_walk_callback(void* context_, kalos_module_parsed parsed, uint16_t index, kalos_module* module, kalos_export* export) {
    struct walk_callback_context* context = context_;
    kalos_state* state = kalos_state_from_run_state(context->state);
    script_current_export = export;
    kalos_value ctx = kalos_value_clone(state, &context->script_context);
    kalos_object_ref obj = NULL;
    switch (export->type) {
        case KALOS_EXPORT_TYPE_FUNCTION:
            obj = kalos_allocate_prop_object(state, NULL, &kalos_module_idl_module_object_function_obj_props);
            script_current_function = kalos_module_get_list_item(parsed, export->entry.function_overload_list.head);
            kalos_module_idl_module_trigger_function(context->state, &ctx, &obj);
            script_current_function = NULL;
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
    // TODO: Watcom takes forever to compile if these aren't static
    static const char IDL_COMPILER_SCRIPT[] = {
        #include "_kalos_idl_compiler.kalos.inc"
    };

    static const char IDL_COMPILER_IDL[] = {
        #include "_kalos_idl_compiler.kidl.inc"
    };
    script_environment = state;
    kalos_module_parsed modules = kalos_idl_parse_module(IDL_COMPILER_IDL, state);
    if (!modules.buffer) {
        printf("ERROR: %s\n", "failed to parse compiler IDL");
        return false;
    }
    kalos_parse_options options = {0};
    kalos_buffer script = kalos_buffer_alloc(state, 8192);
    kalos_parse_result result = kalos_parse(IDL_COMPILER_SCRIPT, modules, options, script.buffer);
    if (result.error) {
        printf("ERROR: %s\n", result.error);
        return false;
    }
    script_modules = parsed_module;
    script_current_header = (kalos_module_header*)parsed_module.buffer;
    kalos_dispatch dispatch = {0};
    dispatch.dispatch_name = kalos_module_idl_dynamic_dispatch;
    kalos_run_state* run_state = kalos_init((const_kalos_script)script.buffer, &dispatch, state);
    kalos_module_idl_trigger_open(run_state, (kalos_module_get_header(parsed_module)->flags & KALOS_MODULE_FLAG_DISPATCH_NAME) != 0);
    kalos_module_idl_trigger_close(run_state);
    kalos_run_free(run_state);
    return true;
}
