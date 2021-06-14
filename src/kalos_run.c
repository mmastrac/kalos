#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../defines.h"
#include "kalos_dump.h"
#include "kalos_module.h"
#include "kalos_parse.h"
#include "kalos_run.h"
#include "kalos_string_format.h"
#include "kalos_string_system.h"
#include "kalos_util.h"

#define KALOS_VAR_SLOT_SIZE 16

static const int8_t kalos_op_input_size[] = {
#define KALOS_OP(x, in, out, args) in, 
#include "kalos_constants.inc"
};
static const int8_t kalos_op_output_size[] = {
#define KALOS_OP(x, in, out, args) out, 
#include "kalos_constants.inc"
};

typedef struct kalos_state_internal {
    kalos_mem_alloc_fn alloc; // part of public interface
    kalos_mem_free_fn free; // part of public interface
    kalos_script* script;
    kalos_fn* fns;
    kalos_dispatch_fn* modules;
    kalos_stack stack;
    kalos_value globals[KALOS_VAR_SLOT_SIZE];
    kalos_value* locals;
    uint16_t pc;
} kalos_state_internal;

void internal_error(kalos_state_internal* state) {
    ASSERT(false);
    LOG("internal error");
}

void type_error(kalos_state_internal* state) {
    ASSERT(false);
    LOG("type error");
}

void value_error(kalos_state_internal* state) {
    ASSERT(false);
    LOG("value error");
}

#define COERCE(typ, NONE, NUMBER, OBJECT, STRING, BOOL) \
static bool coerce_##typ(kalos_state_internal* state, kalos_value* v) {\
    switch (v->type) {\
        case KALOS_VALUE_NONE: {NONE;};break;\
        case KALOS_VALUE_BOOL: {BOOL;};break;\
        case KALOS_VALUE_NUMBER: {NUMBER;};break;\
        case KALOS_VALUE_STRING: {STRING;};break;\
        case KALOS_VALUE_OBJECT: {OBJECT;};break;\
    }\
    v->type=KALOS_VALUE_##typ; return true;\
}

COERCE(BOOL, 
    v->value.number = 0,
    v->value.number = (v->value.number != 0),
    v->value.number = 1,
    kalos_int x = !kalos_string_isempty(state, v->value.string); kalos_string_release(state, v->value.string); v->value.number = x,
    return true)

COERCE(NUMBER, 
    v->value.number = 0,
    return true,
    return false,
    kalos_int x = atoi(kalos_string_c(state, v->value.string)); kalos_string_release(state, v->value.string); v->value.number = x,
    return true)

COERCE(STRING, 
    v->value.string = kalos_string_allocate(state, ""),
    v->value.string = kalos_string_allocate_fmt(state, "%d", v->value.number),
    return false,
    return true,
    v->value.string = kalos_string_allocate(state, v->value.number ? "true" : "false"))

static bool coerce(kalos_state_internal* state, kalos_value* v, kalos_value_type type) {
    bool success;
    switch (type) {
        case KALOS_VALUE_NONE:
            internal_error(state); // impossible
            return false;
        case KALOS_VALUE_OBJECT:
            success = false; break;
        case KALOS_VALUE_NUMBER:
            success = coerce_NUMBER(state, v); break;
        case KALOS_VALUE_BOOL:
            success = coerce_BOOL(state, v); break;
        case KALOS_VALUE_STRING:
            success = coerce_STRING(state, v); break;
    }
    if (!success) {
        type_error(state);
    }
    return success;
}

#define ENSURE_STACK(size) { if (state->stack.stack_index < size) { internal_error(state); } }
#define ENSURE_STACK_SPACE(size) { if (KALOS_STACK_SIZE - state->stack.stack_index - 1 < size) { internal_error(state); } }

static kalos_int op_number_op(kalos_state_internal* state, kalos_op op, kalos_int a, kalos_int b) {
    if (op == KALOS_OP_DIVIDE && b == 0) {
        return 0;
    }
    #define KALOS_OP_C(x, operator) case KALOS_OP_##x: return a operator b;
    switch (op) {
        #include "kalos_constants.inc"
        case KALOS_OP_MINIMUM:
            return min(a, b);
        case KALOS_OP_MAXIMUM:
            return max(a, b);
        default:
            internal_error(state); // impossible
            return 0;
    }
}

static kalos_value op_logical_op(kalos_state_internal* state, kalos_op op, kalos_value* a, kalos_value* b) {
    kalos_value v = kalos_value_clone(state, a);
    coerce(state, &v, KALOS_VALUE_BOOL);
    if (v.value.number ^ (op == KALOS_OP_LOGICAL_AND)) {
        return kalos_value_move(state, a);
    } else {
        return kalos_value_move(state, b);
    }
}

static kalos_int op_compare_string(kalos_state_internal* state, kalos_op op, kalos_string* a, kalos_string* b) {
    if (op < KALOS_OP_EQUAL || op > KALOS_OP_LTE) {
        internal_error(state); // impossible
        return 0;
    }
    return op_number_op(state, op, kalos_string_compare(state, *a, *b), 0);
}

static kalos_int op_compare_type(kalos_state_internal* state, kalos_op op, kalos_value* a, kalos_value* b) {
    if (op == KALOS_OP_EQUAL) {
        return a->type == b->type;
    } else if (op == KALOS_OP_NOT_EQUAL) {
        return a->type != b->type;
    }
    return 0;
}

static kalos_string op_string_add(kalos_state_internal* state, kalos_op op, kalos_string* a, kalos_value* b) {
    coerce(state, b, KALOS_VALUE_STRING);
    return kalos_string_take_append(state, a, &b->value.string);
}

static kalos_string op_string_add2(kalos_state_internal* state, kalos_op op, kalos_value* a, kalos_string* b) {
    coerce(state, a, KALOS_VALUE_STRING);
    return kalos_string_take_append(state, &a->value.string, b);
}

static kalos_string op_string_multiply(kalos_state_internal* state, kalos_op op, kalos_string* a, kalos_int b) {
    if (b <= 0 || kalos_string_isempty(state, *a)) {
        return kalos_string_allocate(state, "");
    }
    return kalos_string_take_repeat(state, a, b);
}

static kalos_string op_string_multiply2(kalos_state_internal* state, kalos_op op, kalos_int a, kalos_string* b) {
    return op_string_multiply(state, op, b, a);
}

static kalos_int op_string_number(kalos_state_internal* state, kalos_op op, kalos_string* v) {
    switch (op) {
        case KALOS_OP_ORD: return kalos_string_char_at(state, *v, 0);
        case KALOS_OP_LENGTH: return kalos_string_length(state, *v);
        default: return 0;
    }
}

static kalos_string op_to_hex_or_char(kalos_state_internal* state, kalos_op op, kalos_int v) {
    if (op == KALOS_OP_TO_HEX) {
        return kalos_string_allocate_fmt(state, "0x%x", v);
    } else if (op == KALOS_OP_TO_CHAR) {
        kalos_writable_string str = kalos_string_allocate_writable_size(state, 1);
        char* s = kalos_string_writable_c(state, str);
        s[0] = v;
        return kalos_string_commit(state, str);
    }
    internal_error(state);
    return kalos_string_allocate(state, "");
}

static kalos_string op_to_string(kalos_state_internal* state, kalos_op op, kalos_value* v) {
    coerce(state, v, KALOS_VALUE_STRING);
    kalos_string s = kalos_string_take(state, &v->value.string);
    return s;
}

static kalos_int op_to_bool(kalos_state_internal* state, kalos_op op, kalos_value* v) {
    coerce(state, v, KALOS_VALUE_BOOL);
    return v->value.number ^ (op == KALOS_OP_LOGICAL_NOT);
}

static kalos_int op_to_int(kalos_state_internal* state, kalos_op op, kalos_value* v) {
    coerce(state, v, KALOS_VALUE_NUMBER);
    return v->value.number;
}

static void op_goto(kalos_state_internal* state, kalos_op op, uint16_t pc) {
    state->pc = pc;
}

static void op_goto_if(kalos_state_internal* state, kalos_op op, kalos_int flag, kalos_int pc) {
    if (flag) {
        state->pc = pc;
    }
}

static kalos_int op_unary_number(kalos_state_internal* state, kalos_op op, kalos_int v) {
    switch (op) {
        case KALOS_OP_NEGATE: return -v;
        case KALOS_OP_POSIVATE: return +v;
        case KALOS_OP_BITWISE_NOT: return ~v;
        case KALOS_OP_ABS: return abs(v);
        case KALOS_OP_LOGICAL_NOT: return !v;
        default: internal_error(state); return v; // impossible
    }
}

struct sized_iterator_context {
    kalos_iterable_fn fn;
    size_t context_size;
    uint16_t index;
    uint16_t count;
};

static kalos_value sized_iternext(kalos_state state, kalos_object_ref* iter, bool* done) {
    struct sized_iterator_context* sized_context = (*iter)->context;
    kalos_value value = {0};
    if (sized_context->index >= sized_context->count) {
        *done = true;
        return value;
    }
    *done = false;
    sized_context->fn(state, (uint8_t*)(*iter)->context + sizeof(struct sized_iterator_context), sized_context->index++, &value);
    return value;
}

static kalos_object_ref sized_iterstart(kalos_state state, kalos_object_ref* range) {
    struct sized_iterator_context* sized_context = (*range)->context;
    kalos_object_ref iter = kalos_allocate_object(state, sized_context->context_size);
    memcpy(iter->context, sized_context, sized_context->context_size);
    iter->iternext = sized_iternext;
    return iter;
}

kalos_object_ref kalos_allocate_sized_iterable(kalos_state state, kalos_iterable_fn fn, size_t context_size, void** context, uint16_t count) {
    // This currently copies the entire context to each iterator which is not as efficient as just
    // retaining the parent, but that requires a bit more complex code and might be slower overall
    kalos_object_ref obj = kalos_allocate_object(state, sizeof(struct sized_iterator_context) + context_size);
    if (context || context_size) {
        *context = (uint8_t*)obj->context + sizeof(struct sized_iterator_context);
    }
    struct sized_iterator_context* sized_context = obj->context;
    sized_context->count = count;
    sized_context->index = 0;
    sized_context->context_size = sizeof(struct sized_iterator_context) + context_size;
    sized_context->fn = fn;
    obj->iterstart = sized_iterstart;
    return obj;
}

struct kalos_range {
    kalos_int current;
    kalos_int start;
    kalos_int end;
    kalos_int step;
};

static void range_iterfunc(kalos_state state, void* context, uint16_t index, kalos_value* value) {
    kalos_int* range_spec = context;
    value->type = KALOS_VALUE_NUMBER;
    value->value.number = range_spec[0] + range_spec[1] * index;
}

static kalos_object_ref op_range(kalos_state_internal* state, kalos_op op, kalos_int start, kalos_int end) {
    kalos_int* range_spec;
    kalos_object_ref range = kalos_allocate_sized_iterable(state, range_iterfunc, sizeof(kalos_int[2]), (void**)&range_spec, end <= start ? 0 : end - start);
    range_spec[0] = start;
    range_spec[1] = 1; // step (TODO)
    return range;
}

struct kalos_split {
    kalos_string splitee;
    kalos_string splitter;
    int offset;
    int length;
    int splitter_length;
};

static void split_free(kalos_state state, kalos_object_ref* object) {
    struct kalos_split* split_context = (struct kalos_split*)(*object)->context;
    kalos_string_release(state, split_context->splitee);
    kalos_string_release(state, split_context->splitter);
}

static kalos_value split_iternext(kalos_state state, kalos_object_ref* iter, bool* done) {
    struct kalos_split* split_context = (struct kalos_split*)(*iter)->context;
    kalos_value value = {0};
    if (split_context->offset > split_context->length) {
        *done = true;
        return value;
    }
    *done = false;

    int found_offset = kalos_string_find_from(state, split_context->splitee, split_context->splitter, split_context->offset);
    value.type = KALOS_VALUE_STRING;
    if (found_offset == -1) {
        // Copy the remainder of the string, or an empty string if nothing is left
        value.value.string = kalos_string_take_substring_start(state, &split_context->splitee, split_context->offset);
        split_context->offset = split_context->length + 1;
    } else {
        kalos_string splitee = kalos_string_duplicate(state, split_context->splitee);
        value.value.string = kalos_string_take_substring(state, &splitee, split_context->offset, found_offset - split_context->offset);
        split_context->offset += (found_offset - split_context->offset) + split_context->splitter_length;
    }
    VALIDATE_STRING(value.value.string);
    return value;
}

static kalos_object_ref split_iterstart(kalos_state state, kalos_object_ref* split) {
    struct kalos_split* split_context = (struct kalos_split*)(*split)->context;
    kalos_object_ref iter = kalos_allocate_object(state, sizeof(struct kalos_split));
    struct kalos_split* iter_context = (struct kalos_split*)iter->context;
    *iter_context = *split_context;
    split_context->splitee = kalos_string_duplicate(state, split_context->splitee);
    split_context->splitter = kalos_string_duplicate(state, split_context->splitter);
    iter->iternext = split_iternext;
    iter->object_free = split_free;
    return iter;
}

static kalos_object_ref op_split(kalos_state_internal* state, kalos_op op, kalos_string* splitee, kalos_string* splitter) {
    kalos_object_ref split = kalos_allocate_object(state, sizeof(struct kalos_split));
    struct kalos_split* split_context = (struct kalos_split*)split->context;
    split_context->length = kalos_string_length(state, *splitee);
    split_context->splitee = kalos_string_take(state, splitee);
    split_context->splitter_length = kalos_string_length(state, *splitter);
    split_context->splitter = kalos_string_take(state, splitter);
    split->iterstart = split_iterstart;
    split->object_free = split_free;
    return split;
}

static kalos_object_ref op_iterator(kalos_state_internal* state, kalos_op op, kalos_object_ref* iterable) {
    return (*iterable)->iterstart(state, iterable);
}

static inline void op_drop(kalos_state_internal* state, kalos_op op, kalos_value* value) {}

static kalos_string op_push_string(kalos_state_internal* state, kalos_op op) {
    kalos_string string = kalos_string_allocate(state, (const char*)&state->script->script_ops[state->pc]);
    LOG("push: %s", kalos_string_c(state, string));
    state->pc += kalos_string_length(state, string) + 1;
    return string;
}

static kalos_int op_push_integer(kalos_state_internal* state, kalos_op op) {
    kalos_int int_value;
    memcpy(&int_value, &state->script->script_ops[state->pc], sizeof(kalos_int));
    state->pc += sizeof(kalos_int);
    return int_value;
}

static kalos_int op_push_bool(kalos_state_internal* state, kalos_op op) {
    return op == KALOS_OP_PUSH_TRUE;
}

static kalos_string op_format(kalos_state_internal* state, kalos_op op, kalos_value* v) {
    kalos_string_format* format = (kalos_string_format*)&state->script->script_ops[state->pc];
    state->pc += sizeof(kalos_string_format);
    if (v->type == KALOS_VALUE_NUMBER) {
        return kalos_string_format_int(state, v->value.number, format);
    } else if (v->type == KALOS_VALUE_STRING) {
        int size = snprintf(NULL, 0, "%*.*s", format->min_width, format->precision ? format->precision : 0xffff, kalos_string_c(state, v->value.string));
        kalos_writable_string str = kalos_string_allocate_writable_size(state, size);
        char* s = kalos_string_writable_c(state, str);
        sprintf(s, "%*s", format->min_width, kalos_string_c(state, v->value.string));
        return kalos_string_commit(state, str);
    }
    return kalos_string_allocate(state, "");
}

static void op_store(kalos_state_internal* state, kalos_op op, kalos_value* v, kalos_int slot) {
    kalos_value* storage = op == KALOS_OP_STORE_LOCAL ? state->locals : state->globals;
    kalos_value_move_to(state, v, &storage[slot]);
}

static kalos_value op_load(kalos_state_internal* state, kalos_op op, kalos_int slot) {
    kalos_value* storage = op == KALOS_OP_LOAD_LOCAL ? state->locals : state->globals;
    return kalos_value_clone(state, &storage[slot]);
}

kalos_state kalos_init(kalos_script* script, kalos_dispatch_fn* modules, kalos_fn* fns) {
    kalos_state_internal* state = fns->alloc(sizeof(kalos_state_internal));
    if (!state) {
        fns->error("malloc");
    }
    memset(state, 0, sizeof(kalos_state_internal));
    state->alloc = fns->alloc;
    state->free = fns->free;
    state->modules = modules;
    state->script = script;
    state->fns = fns;
    LOG("%s", "Triggering global");
    kalos_trigger((kalos_state)state, kalos_make_address(-1, -1));
    return (kalos_state)state;
}

kalos_state kalos_init_for_test(kalos_fn* fns) {
    kalos_state_internal* state = fns->alloc(sizeof(kalos_state_internal));
    if (!state) {
        fns->error("malloc");
    }
    memset(state, 0, sizeof(kalos_state_internal));
    state->alloc = fns->alloc;
    state->free = fns->free;
    state->fns = fns;
    return (kalos_state)state;
}

void kalos_load_arg_any(kalos_state state_, kalos_int index, kalos_value* arg) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    kalos_value_move_to(state, arg, &state->stack.stack[state->stack.stack_index + index]);
}

void kalos_load_arg_object(kalos_state state_, kalos_int index, kalos_object_ref* arg) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    state->stack.stack[state->stack.stack_index + index].type = KALOS_VALUE_OBJECT;
    state->stack.stack[state->stack.stack_index + index].value.object = kalos_object_take(state, arg);
}

void kalos_load_arg_number(kalos_state state_, kalos_int index, kalos_int arg) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    state->stack.stack[state->stack.stack_index + index].type = KALOS_VALUE_NUMBER;
    state->stack.stack[state->stack.stack_index + index].value.number = arg;
}

void kalos_load_arg_string(kalos_state state_, kalos_int index, kalos_string* arg) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    state->stack.stack[state->stack.stack_index + index].type = KALOS_VALUE_STRING;
    state->stack.stack[state->stack.stack_index + index].value.string = kalos_string_take(state, arg);
}

void kalos_load_arg_bool(kalos_state state_, kalos_int index, bool arg) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    state->stack.stack[state->stack.stack_index + index].type = KALOS_VALUE_BOOL;
    state->stack.stack[state->stack.stack_index + index].value.number = arg;
}

void kalos_trigger(kalos_state state_, kalos_export_address handle_address) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    kalos_section_header* header;
    int original_stack_index = state->stack.stack_index;
    int original_pc = state->pc;
    kalos_value* original_locals = state->locals;
    state->pc = kalos_find_section(state->script, handle_address, &header);
    if (state->pc == 0) {
        goto done;
    }
    state->locals = &state->stack.stack[original_stack_index];
    state->stack.stack_index += header->locals_size;
    for (;;) {
        if (state->pc >= state->script->script_buffer_size) {
            state->fns->error("Internal error");
            goto done;
        }
        kalos_op op = state->script->script_ops[state->pc++];
        kalos_value *v1, *v2;
        if (op >= KALOS_OP_MAX || state->stack.stack_index < 0) {
            state->fns->error("Internal error");
            goto done;
        }
        LOG("PC %04x exec %s (stack = %d)", state->pc - 1, kalos_op_strings[op], state->stack.stack_index);
        int stack_index = state->stack.stack_index;
        if (kalos_op_input_size[op] != -1) {
            ENSURE_STACK(kalos_op_input_size[op]);
            if (kalos_op_output_size[op] != -1 && kalos_op_output_size[op] > kalos_op_input_size[op]) {
                ENSURE_STACK_SPACE(kalos_op_output_size[op] - kalos_op_input_size[op]);
            }
            state->stack.stack_index -= kalos_op_input_size[op];
        }
        switch (op) {
            case KALOS_OP_END:
                goto done;
            case KALOS_OP_DEBUGGER: 
                // Set breakpoints here
                break;
            case KALOS_OP_DUP: {
                kalos_value* v = peek(&state->stack, 0);
                kalos_value_clone_to(state, v, push_raw(&state->stack));
                break;
            }
            case KALOS_OP_ITERATOR_NEXT: {
                state->stack.stack_index++; // push the iterator back on the stack
                kalos_value* iterator = peek(&state->stack, 0);
                bool done;
                kalos_value next = iterator->value.object->iternext(state_, &iterator->value.object, &done);
                push_bool(&state->stack, done);
                kalos_value_move_to(state, &next, push_raw(&state->stack));
                break;
            }
            case KALOS_OP_CALL_NORET:
            case KALOS_OP_CALL: {
                int export = pop(&state->stack)->value.number;
                int module = pop(&state->stack)->value.number;
                LOG("%d:%d %p", export, module, state->modules[module]);
                state->modules[module](state_, export, &state->stack, op == KALOS_OP_CALL);
                break;
            }
            case KALOS_OP_MAKE_LIST: {
                int count = kalos_stack_vararg_count(&state->stack);
                kalos_object_ref list = kalos_allocate_list(state, count, kalos_stack_vararg_start(&state->stack, count));
                kalos_stack_cleanup(count + 1, state, &state->stack);
                push_object(&state->stack, list);
                break;
            }
            case KALOS_OP_GETPROP: {
                int prop = pop(&state->stack)->value.number;
                kalos_object_ref object = pop(&state->stack)->value.object;
                if (!object->dispatch || !object->dispatch(state, &object, prop, &state->stack)) {
                    value_error(state);
                }
                kalos_object_release(state, &object);
                break;
            }
            case KALOS_OP_SETPROP: {
                int prop = pop(&state->stack)->value.number;
                kalos_value* value = pop(&state->stack);
                kalos_object_ref object = pop(&state->stack)->value.object;
                *push_raw(&state->stack) = *value;
                if (!object->dispatch || !object->dispatch(state, &object, prop, &state->stack)) {
                    value_error(state);
                }
                kalos_object_release(state, &object);
                break;
            }

#define is_NUMBER(index) (peek(&state->stack, -index-1)->type == KALOS_VALUE_NUMBER || peek(&state->stack, -index-1)->type == KALOS_VALUE_BOOL)
#define is_BOOL(index) (peek(&state->stack, -index-1)->type == KALOS_VALUE_BOOL)
#define is_STRING(index) (peek(&state->stack, -index-1)->type == KALOS_VALUE_STRING)
#define is_OBJECT(index) (peek(&state->stack, -index-1)->type == KALOS_VALUE_OBJECT)
#define is_ANY(...) true
#define is_VOID(...) true

#define arg_ANY(x) , &state->stack.stack[state->stack.stack_index+x]
#define save_ANY kalos_value x = 
#define push_ANY state->stack.stack[state->stack.stack_index++] = x;
#define cleanup_ANY(x) kalos_clear(state, &state->stack.stack[state->stack.stack_index + x]); \

#define arg_NUMBER(x) , state->stack.stack[state->stack.stack_index+x].value.number
#define save_NUMBER kalos_int x = 
#define push_NUMBER state->stack.stack[state->stack.stack_index].type = KALOS_VALUE_NUMBER; state->stack.stack[state->stack.stack_index++].value.number = x;
#define cleanup_NUMBER(x)

#define arg_STRING(x) , &state->stack.stack[state->stack.stack_index+x].value.string
#define save_STRING kalos_string s = 
#define push_STRING state->stack.stack[state->stack.stack_index].type = KALOS_VALUE_STRING; state->stack.stack[state->stack.stack_index++].value.string = s;
#define cleanup_STRING(x) kalos_string_release(state, state->stack.stack[state->stack.stack_index+x].value.string);

#define arg_OBJECT(x) , &state->stack.stack[state->stack.stack_index+x].value.object
#define save_OBJECT kalos_object_ref o = 
#define push_OBJECT state->stack.stack[state->stack.stack_index].type = KALOS_VALUE_OBJECT; state->stack.stack[state->stack.stack_index++].value.object = o;
#define cleanup_OBJECT(x) kalos_object_release(state, &state->stack.stack[state->stack.stack_index+x].value.object);

#define arg_BOOL(x) , state->stack.stack[state->stack.stack_index+x].value.number
#define save_BOOL bool b = 
#define push_BOOL state->stack.stack[state->stack.stack_index].type = KALOS_VALUE_BOOL; state->stack.stack[state->stack.stack_index++].value.number = b;
#define cleanup_BOOL(x)

#define arg_VOID(x)
#define save_VOID 
#define push_VOID state->stack.stack[state->stack.stack_index].type = KALOS_VALUE_NONE;
#define cleanup_VOID(x)

#define OP(op, ...) \
    OPS op \
        if (0) { /* ignored */ } OP_CASES(__VA_ARGS__) else { value_error(state); } break;
#define OPS(...) KALOS_FOREACH__(OP_OP_CASE, __VA_ARGS__)
#define OP_OP_CASE(index_, op) KALOS_CONCAT__(OP_OP_CASE, KM__PRIMITIVE_COMPARE(op,))(op)
#define OP_OP_CASE0(op) case KALOS_OP_##op:
#define OP_OP_CASE1(op) 
#define OP_CASES(...) KALOS_FOREACH__(OP_CASE, __VA_ARGS__)
#define OP_CASE(index_, tuple_) OP_CASE_ tuple_ (,)
#define OP_CASE_(a, ...) KALOS_CONCAT__(OP_CASE, KM__PRIMITIVE_COMPARE(a,))(a,__VA_ARGS__)
#define OP_CASE0(method_, return_, ...) \
    else if ( KALOS_FOREACH2__(OP_TYPE_CASE, __VA_ARGS__) true ) { save_##return_ method_(state, op KALOS_FOREACH2__(OP_TYPE_ARG, __VA_ARGS__)); KALOS_FOREACH2__(OP_CLEANUP_ARG, __VA_ARGS__) push_##return_ } \
    OP_EAT_PARENS
#define OP_CASE1(...)
#define OP_TYPE_CASE(a, type) KALOS_CONCAT__(OP_, KM__PRIMITIVE_COMPARE(type,))(a, type)
#define OP_0(index, x) is_##x(index) &&
#define OP_1(...)
#define OP_TYPE_ARG(a, type) KALOS_CONCAT__(OP_ARG_, KM__PRIMITIVE_COMPARE(type,))(a, type)
#define OP_ARG_0(index, x) arg_##x(index)
#define OP_ARG_1(...)
#define OP_CLEANUP_ARG(a, type) KALOS_CONCAT__(OP_CLEANUP_, KM__PRIMITIVE_COMPARE(type,))(a, type)
#define OP_CLEANUP_0(index, x) cleanup_##x(index)
#define OP_CLEANUP_1(...)
#define OP_EAT_PARENS(...)

            OP((DROP),        (op_drop, VOID, ANY));
            OP((PUSH_STRING), (op_push_string, STRING, VOID));
            OP((PUSH_INTEGER),(op_push_integer, NUMBER, VOID));
            OP((PUSH_TRUE, PUSH_FALSE),
                              (op_push_bool, BOOL, VOID));
            OP((GOTO),        (op_goto, VOID, NUMBER));
            OP((GOTO_IF),     (op_goto_if, VOID, BOOL, NUMBER));
            OP((LOAD_LOCAL, LOAD_GLOBAL),
                              (op_load, ANY, NUMBER));
            OP((STORE_LOCAL, STORE_GLOBAL),
                              (op_store, VOID, ANY, NUMBER));
            OP((FORMAT),      (op_format, STRING, ANY));
            OP((LENGTH, ORD), (op_string_number, NUMBER, STRING));
            OP((TO_STRING),   (op_to_string, STRING, ANY));
            OP((TO_BOOL),     (op_to_bool, BOOL, ANY));
            OP((LOGICAL_NOT), (op_unary_number, BOOL, NUMBER), (op_to_bool, BOOL, ANY));
            OP((TO_INT),      (op_to_int, NUMBER, ANY));
            OP((TO_CHAR),     (op_to_hex_or_char, STRING, NUMBER));
            OP((TO_HEX),      (op_to_hex_or_char, STRING, NUMBER));
            OP((ITERATOR),    (op_iterator, OBJECT, OBJECT));
            OP((SPLIT),       (op_split, OBJECT, STRING, STRING));
            OP((RANGE),       (op_range, OBJECT, NUMBER, NUMBER));
            // Compare ops with numeric and string equivalents
            OP((EQUAL, NOT_EQUAL),
                              (op_number_op, BOOL, NUMBER, NUMBER), (op_compare_string, BOOL, STRING, STRING), (op_compare_type, BOOL, ANY, ANY));
            OP((GT, GTE, LT, LTE),
                              (op_number_op, BOOL, NUMBER, NUMBER), (op_compare_string, BOOL, STRING, STRING));
            // Add and multiply work with strings/numbers
            OP((ADD),         (op_number_op, NUMBER, NUMBER, NUMBER), (op_string_add, STRING, STRING, ANY), (op_string_add2, STRING, ANY, STRING));
            OP((MULTIPLY),    (op_number_op, NUMBER, NUMBER, NUMBER), (op_string_multiply, STRING, STRING, NUMBER), (op_string_multiply2, STRING, NUMBER, STRING));
            // Unary ops
            OP((NEGATE, POSIVATE, BITWISE_NOT, ABS), 
                              (op_unary_number, NUMBER, NUMBER));
            // Purely boolean binary ops
            OP((LOGICAL_AND, LOGICAL_OR),
                              (op_number_op, NUMBER, BOOL, BOOL), (op_logical_op, ANY, ANY, ANY));
            // Purely numeric binary ops
            OP((MINIMUM, MAXIMUM, LEFT_SHIFT, RIGHT_SHIFT, BIT_AND, BIT_OR, BIT_XOR, SUBTRACT, DIVIDE, MODULUS),
                              (op_number_op, NUMBER, NUMBER, NUMBER));

            case KALOS_OP_MAX:
                internal_error(state); // impossible
                break;
        }

        // Confirm stack is correct
        if (kalos_op_input_size[op] != -1 && kalos_op_output_size[op] != -1) {
            int diff = state->stack.stack_index - stack_index;
            ASSERT(diff == kalos_op_output_size[op] - kalos_op_input_size[op]);
        }
    }

    done:
    for (int i = original_stack_index + 1; i < state->stack.stack_index; i++) {
        kalos_clear(state, &state->stack.stack[i]);
    }
    state->stack.stack_index = original_stack_index;
    state->pc = original_pc;
    state->locals = original_locals;
}

void kalos_run_free(kalos_state state_) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    for (int i = 0; i < KALOS_VAR_SLOT_SIZE; i++) {
        kalos_clear(state, &state->globals[i]);
    }
    state->fns->free(state);
}

struct list_iter_context {
    kalos_object_ref object;
    kalos_int index;
};

kalos_value list_iternext(kalos_state state, kalos_object_ref* object, bool* done) {
    struct list_iter_context* context = (*object)->context;
    kalos_value* array = context->object->context;
    if (context->index >= array[0].value.number) {
        *done = true;
        kalos_object_release(state, &context->object);
        kalos_value none = {0};
        return none;
    }
    *done = false;
    return kalos_value_clone(state, &array[1 + context->index++]);
}

kalos_object_ref list_iterstart(kalos_state state, kalos_object_ref* object) {
    kalos_object_ref iter = kalos_allocate_object(state, sizeof(struct list_iter_context));
    struct list_iter_context* context = iter->context;
    kalos_object_retain(state, *object);
    context->index = 0;
    context->object = *object;
    iter->iternext = list_iternext;
    return iter;
}

void list_free(kalos_state state_, kalos_object_ref* object) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    kalos_value* array = (*object)->context;
    for (int i = 0; i < array[0].value.number; i++) {
        kalos_clear(state, &array[i + 1]);
    }
    state->fns->free(array);
}

kalos_object_ref kalos_allocate_list(kalos_state state_, kalos_int size, kalos_value* values) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    kalos_object_ref object = kalos_allocate_object(state, 0);
    object->context = state->fns->alloc(sizeof(kalos_value) * (size + 1)); // allocate size of size
    kalos_value* array = object->context;
    array[0].type = KALOS_VALUE_NUMBER;
    array[0].value.number = size;
    memcpy(array + 1, values, sizeof(kalos_value) * size);
    for (int i = 0; i < size; i++) {
        values[i].type = KALOS_VALUE_NONE;
    }
    object->iterstart = list_iterstart;
    object->object_free = list_free;
    return object;
}

kalos_object_ref kalos_allocate_object(kalos_state state_, size_t context_size) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    kalos_object_ref object = state->fns->alloc(sizeof(kalos_object) + context_size);
    memset(object, 0, sizeof(*object) + context_size);
    if (context_size) {
        object->context = (uint8_t*)object + sizeof(kalos_object);
    }
    return object;
}

kalos_object_ref kalos_allocate_prop_object(kalos_state state, void* context, kalos_object_dispatch dispatch) {
    kalos_object_ref object = kalos_allocate_object(state, 0);
    object->context = context;
    object->dispatch = dispatch;
    return object;
}

void* kaloc_mem_alloc(kalos_state state_, size_t size) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    return state->fns->alloc(size);
}
