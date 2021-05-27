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

#define KALOS_FIELD_NUMBER(v) v->value.number
#define KALOS_FIELD_BOOL(v) v->value.number
#define KALOS_FIELD_STRING(v) &v->value.string
#define KALOS_FIELD_OBJECT(v) v->value.object

#define KALOS_VAR_SLOT_SIZE 16

typedef struct kalos_state_internal {
    kalos_mem_alloc_fn alloc; // part of public interface
    kalos_mem_free_fn free; // part of public interface
    kalos_script* script;
    kalos_fn* fns;
    kalos_module** modules;
    kalos_stack stack;
    kalos_value globals[KALOS_VAR_SLOT_SIZE];
    kalos_value locals[KALOS_VAR_SLOT_SIZE];
    uint16_t pc;
} kalos_state_internal;

#define push_OBJECT push_object
#define push_BOOL push_bool
#define push_STRING push_string
#define push_NUMBER push_number
#define push_NONE push_none
#define PUSH(type_, value_) push_##type_(&state->stack, value_)

#define COERCE(typ, NONE, NUMBER, OBJECT, STRING, BOOL) \
bool coerce_##typ(kalos_state_internal* state, kalos_value* v) {\
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
    v->value.number = !kalos_string_isempty(state, v->value.string),
    return true)

COERCE(NUMBER, 
    v->value.number = 0,
    return true,
    return false,
    v->value.number = atoi(kalos_string_c(state, v->value.string)),
    return true)

COERCE(STRING, 
    v->value.string = kalos_string_allocate(state, ""),
    v->value.string = kalos_string_allocate_fmt(state, "%d", v->value.number),
    return false,
    return true,
    v->value.string = kalos_string_allocate(state, v->value.number ? "true" : "false"))

void internal_error(kalos_state_internal* state) {
}

void type_error(kalos_state_internal* state) {
}

bool coerce(kalos_state_internal* state, kalos_value* v, kalos_value_type type) {
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

kalos_value_type get_major_type(kalos_value* v) {
    if (v->type == KALOS_VALUE_STRING || v->type == KALOS_VALUE_OBJECT) {
        return v->type;
    }
    return KALOS_VALUE_NUMBER;
}

#define POP1(method, typ) \
    (v1 = pop(&state->stack), \
    coerce(state, v1, KALOS_VALUE_##typ), \
    method(state, op, KALOS_FIELD_##typ(v1)))

#define POP2(method, type1, type2) \
    (v2 = pop(&state->stack), \
    v1 = pop(&state->stack), \
    coerce(state, v1, KALOS_VALUE_##type1), \
    coerce(state, v2, KALOS_VALUE_##type2), \
    method(state, op, KALOS_FIELD_##type1(v1), KALOS_FIELD_##type2(v2)))

#define ENSURE_STACK(size) { if (state->stack.stack_index < KALOS_STACK_SIZE) { internal_error(state); } }
#define ENSURE_STACK_SPACE(size) { if (KALOS_STACK_SIZE - state->stack.stack_index - 1 < size) { internal_error(state); } }

kalos_int op_number_op(kalos_state_internal* state, kalos_op op, kalos_int a, kalos_int b) {
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

kalos_int op_string_compare_op(kalos_state_internal* state, kalos_op op, kalos_string* a, kalos_string* b) {
    if (op < KALOS_OP_EQUAL || op > KALOS_OP_LTE) {
        internal_error(state); // impossible
        return 0;
    }
    return op_number_op(state, op, kalos_string_compare(state, *a, *b), 0);
}

kalos_string op_string_add(kalos_state_internal* state, kalos_op op, kalos_string* a, kalos_string* b) {
    return kalos_string_take_append(state, a, b);
}

kalos_string op_string_multiply(kalos_state_internal* state, kalos_op op, kalos_string* a, kalos_int b) {
    if (b <= 0) {
        return kalos_string_allocate(state, "");
    }
    return kalos_string_take_repeat(state, a, b);
}

kalos_string op_string_format(kalos_state_internal* state, kalos_op op, kalos_value* v, kalos_string_format* format) {
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

bool op_bool(kalos_state_internal* state, kalos_op op, bool v) {
    return v;
}

kalos_string op_strings(kalos_state_internal* state, kalos_op op, kalos_string* v) {
    return *v;
}

kalos_int op_string_number(kalos_state_internal* state, kalos_op op, kalos_string* v) {
    switch (op) {
        case KALOS_OP_ORD: return kalos_string_char_at(state, *v, 0);
        case KALOS_OP_LENGTH: return kalos_string_length(state, *v);
        default: return 0;
    }
}

kalos_string op_to_hex_or_char(kalos_state_internal* state, kalos_op op, kalos_int v) {
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

kalos_int op_to_number(kalos_state_internal* state, kalos_op op, kalos_int v) {
    return v;
}

void op_goto(kalos_state_internal* state, kalos_op op, uint16_t pc) {
    state->pc = pc;
}

void op_goto_if(kalos_state_internal* state, kalos_op op, kalos_int flag, kalos_int pc) {
    if (flag) {
        state->pc = pc;
    }
}

kalos_int op_unary_number(kalos_state_internal* state, kalos_op op, kalos_int v) {
    switch (op) {
        case KALOS_OP_NEGATE: return -v;
        case KALOS_OP_POSIVATE: return +v;
        case KALOS_OP_BITWISE_NOT: return ~v;
        case KALOS_OP_LOGICAL_NOT: return !v;
        case KALOS_OP_ABS: return abs(v);
        default: internal_error(state); return v; // impossible
    }
}

struct kalos_range {
    kalos_int current;
    kalos_int start;
    kalos_int end;
    kalos_int step;
};

kalos_value range_iternext(kalos_state state, kalos_object* iter, bool* done) {
    struct kalos_range* range_context = (struct kalos_range*)iter->context;
    kalos_value value = {0};
    if (range_context->current >= range_context->end) {
        *done = true;
        return value;
    }
    value.type = KALOS_VALUE_NUMBER;
    value.value.number = range_context->current;
    range_context->current += range_context->step;
    LOG("range %d %d (curr %d)", range_context->start, range_context->end, range_context->current);
    return value;
}

kalos_object* range_iterstart(kalos_state state, kalos_object* range) {
    struct kalos_range* range_context = (struct kalos_range*)range->context;
    kalos_object* iter = kalos_allocate_object(state, sizeof(struct kalos_range));
    *(struct kalos_range*)iter->context = *range_context;
    iter->iternext = range_iternext;
    return iter;
}

kalos_object* op_range(kalos_state_internal* state, kalos_op op, kalos_int start, kalos_int end) {
    kalos_object* range = kalos_allocate_object(state, sizeof(struct kalos_range));
    struct kalos_range* range_context = (struct kalos_range*)range->context;
    range_context->start = start;
    range_context->end = end;
    range_context->step = 1;
    range->iterstart = range_iterstart;
    return range;
}

struct kalos_split {
    kalos_string splitee;
    kalos_string splitter;
    int offset;
    int length;
    int splitter_length;
};

kalos_value split_iternext(kalos_state state, kalos_object* iter, bool* done) {
    struct kalos_split* split_context = (struct kalos_split*)iter->context;
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
        value.value.string = kalos_string_take_substring(state, &split_context->splitee, split_context->offset, found_offset - split_context->offset);
        split_context->offset += (found_offset - split_context->offset) + split_context->splitter_length;
    }

    return value;
}

kalos_object* split_iterstart(kalos_state state, kalos_object* split) {
    struct kalos_split* split_context = (struct kalos_split*)split->context;
    kalos_object* iter = kalos_allocate_object(state, sizeof(struct kalos_split));
    struct kalos_split* iter_context = (struct kalos_split*)iter->context;
    *iter_context = *split_context;
    iter->iternext = split_iternext;
    return iter;
}

kalos_object* op_split(kalos_state_internal* state, kalos_op op, kalos_string* splitee, kalos_string* splitter) {
    kalos_object* split = kalos_allocate_object(state, sizeof(struct kalos_split));
    struct kalos_split* split_context = (struct kalos_split*)split->context;
    split_context->splitee = *splitee;
    split_context->length = kalos_string_length(state, *splitee);
    split_context->splitter = *splitter;
    split_context->splitter_length = kalos_string_length(state, *splitter);
    split->iterstart = split_iterstart;
    return split;
}

kalos_object* op_iterator(kalos_state_internal* state, kalos_op op, kalos_object* iterable) {
    return iterable->iterstart(state, iterable);
}

kalos_state kalos_init(kalos_script* script, kalos_module** modules, kalos_fn* fns) {
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
    kalos_trigger((kalos_state)state, "");
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

void kalos_trigger(kalos_state state_, char* handler) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    state->pc = kalos_find_section(state->script, handler);
    if (state->pc == 0) {
        return;
    }
    for (;;) {
        if (state->pc >= state->script->script_buffer_size) {
            state->fns->error("Internal error");
            return;
        }
        kalos_op op = state->script->script_ops[state->pc++];
        kalos_value *v1, *v2;
        if (op >= KALOS_OP_MAX || state->stack.stack_index < 0) {
            state->fns->error("Internal error");
            return;
        }
        LOG("exec %s (stack = %d)", kalos_op_strings[op], state->stack.stack_index);
        switch (op) {
            case KALOS_OP_END:
                return;
            case KALOS_OP_DROP:
                ENSURE_STACK(1);
                pop(&state->stack);
                break;
            case KALOS_OP_DEBUGGER: 
                // Set breakpoints here
                break;
            case KALOS_OP_PUSH_STRING: {
                ENSURE_STACK_SPACE(1);
                kalos_string string = kalos_string_allocate(state, (const char*)&state->script->script_ops[state->pc]);
                push_string(&state->stack, string);
                state->pc += kalos_string_length(state, string) + 1;
                break;
            }
            case KALOS_OP_PUSH_INTEGER: {
                ENSURE_STACK_SPACE(1);
                kalos_int* int_value = (kalos_int*)&state->script->script_ops[state->pc];
                push_number(&state->stack, *int_value);
                state->pc += sizeof(kalos_int);
                break;
            }
            case KALOS_OP_FORMAT:
                ENSURE_STACK(1);
                kalos_string_format* string_format = (kalos_string_format*)&state->script->script_ops[state->pc];
                state->pc += sizeof(kalos_string_format);
                v1 = pop(&state->stack);
                PUSH(STRING, op_string_format(state, op, v1, string_format));
                break;
            case KALOS_OP_LENGTH:
            case KALOS_OP_ORD:
                PUSH(NUMBER, POP1(op_string_number, STRING));
                break;
            case KALOS_OP_TO_STRING:
                PUSH(STRING, POP1(op_strings, STRING));
                break;
            case KALOS_OP_TO_BOOL:
                PUSH(BOOL, POP1(op_bool, BOOL));
                break;
            case KALOS_OP_TO_INT:
                PUSH(NUMBER, POP1(op_to_number, NUMBER));
                break;
            case KALOS_OP_TO_CHAR:
            case KALOS_OP_TO_HEX:
                PUSH(STRING, POP1(op_to_hex_or_char, NUMBER));
                break;
            case KALOS_OP_GOTO:
                POP1(op_goto, NUMBER);
                break;
            case KALOS_OP_GOTO_IF:
                POP2(op_goto_if, BOOL, NUMBER);
                break;
            case KALOS_OP_ITERATOR:
                PUSH(OBJECT, POP1(op_iterator, OBJECT));
                break;
            case KALOS_OP_ITERATOR_NEXT: {
                kalos_value* iterator = peek(&state->stack, 0);
                bool done;
                kalos_value next = iterator->value.object->iternext(state_, iterator->value.object, &done);
                push_bool(&state->stack, done);
                *push_raw(&state->stack) = next;
                break;
            }
            case KALOS_OP_NEGATE:
            case KALOS_OP_POSIVATE:
            case KALOS_OP_BITWISE_NOT:
            case KALOS_OP_ABS:
                PUSH(NUMBER, POP1(op_unary_number, NUMBER));
                break;
            case KALOS_OP_LOGICAL_NOT:
                PUSH(BOOL, POP1(op_unary_number, BOOL));
                break;
            case KALOS_OP_CALL: {
                ENSURE_STACK(2);
                int export = pop(&state->stack)->value.number;
                int module = pop(&state->stack)->value.number;
                state->modules[module]->dispatch(state_, export, &state->stack);
                break;
            }
            case KALOS_OP_GETPROP:
                break;
            case KALOS_OP_LOAD_GLOBAL:
            case KALOS_OP_STORE_GLOBAL:
            case KALOS_OP_LOAD_LOCAL:
            case KALOS_OP_STORE_LOCAL: {
                ENSURE_STACK(1);
                int slot = pop(&state->stack)->value.number;
                kalos_value* storage = (op == KALOS_OP_STORE_GLOBAL || op == KALOS_OP_LOAD_GLOBAL) ? state->locals : state->globals;
                if (op == KALOS_OP_STORE_GLOBAL || op == KALOS_OP_STORE_LOCAL) {
                    ENSURE_STACK(1);
                    if (storage[slot].type == KALOS_VALUE_STRING) {
                        kalos_string_release(state, storage[slot].value.string);
                    }
                    storage[slot] = *pop(&state->stack);
                } else {
                    kalos_value* stack = push_raw(&state->stack);
                    if (storage[slot].type == KALOS_VALUE_STRING) {
                        stack->type = KALOS_VALUE_STRING;
                        stack->value.string = kalos_string_duplicate(state, storage[slot].value.string);
                    } else {
                        *stack = storage[slot];
                    }
                }
                break;
            }
            case KALOS_OP_EQUAL:
            case KALOS_OP_NOT_EQUAL:
            case KALOS_OP_GT:
            case KALOS_OP_LT:
            case KALOS_OP_GTE:
            case KALOS_OP_LTE: {
                ENSURE_STACK(2);
                v1 = pop(&state->stack);
                v2 = pop(&state->stack);
                int major_type = max(get_major_type(v1), get_major_type(v2));
                if (major_type == KALOS_VALUE_OBJECT) {
                    bool eq = (op == KALOS_OP_EQUAL), ne = (op == KALOS_OP_NOT_EQUAL);
                    push_bool(&state->stack, 
                        ((ne || eq) && v1->type == KALOS_VALUE_OBJECT && v1->type == KALOS_VALUE_OBJECT && v1->value.object == v2->value.object) ^ (op == ne));
                    break;
                }
                coerce(state, v1, major_type);
                coerce(state, v2, major_type);
                if (major_type == KALOS_VALUE_NUMBER) {
                    PUSH(NUMBER, op_number_op(state, op, v2->value.number, v1->value.number));
                } else if (major_type == KALOS_VALUE_STRING) {
                    PUSH(NUMBER, op_string_compare_op(state, op, &v2->value.string, &v1->value.string));
                }
                break;
            }
            case KALOS_OP_ADD:
                ENSURE_STACK(2);
                v1 = peek(&state->stack, 0);
                v2 = peek(&state->stack, 1);
                int major_type = max(get_major_type(v1), get_major_type(v2));
                if (major_type == KALOS_VALUE_STRING) {
                    PUSH(STRING, POP2(op_string_add, STRING, STRING));
                } else {
                    PUSH(NUMBER, POP2(op_number_op, NUMBER, NUMBER));
                }
                break;
            case KALOS_OP_MULTIPLY:
                ENSURE_STACK(2);
                if (peek(&state->stack, 0)->type == KALOS_VALUE_STRING) {
                    // ...
                    PUSH(STRING, POP2(op_string_multiply, STRING, NUMBER));
                } else {
                    PUSH(NUMBER, POP2(op_number_op, NUMBER, NUMBER));
                }
                break;
            // Purely boolean binary ops
            case KALOS_OP_LOGICAL_AND:
            case KALOS_OP_LOGICAL_OR:
                PUSH(BOOL, POP2(op_number_op, BOOL, BOOL));
                break;
            // Purely numeric binary ops
            case KALOS_OP_MINIMUM:
            case KALOS_OP_MAXIMUM:
            case KALOS_OP_LEFT_SHIFT:
            case KALOS_OP_RIGHT_SHIFT:
            case KALOS_OP_BIT_AND:
            case KALOS_OP_BIT_OR:
            case KALOS_OP_BIT_XOR:
            case KALOS_OP_SUBTRACT: 
            case KALOS_OP_DIVIDE:
            case KALOS_OP_MODULUS:
                PUSH(NUMBER, POP2(op_number_op, NUMBER, NUMBER));
                break;
            case KALOS_OP_SPLIT:
                PUSH(OBJECT, POP2(op_split, STRING, STRING));
                break;
            case KALOS_OP_RANGE:
                PUSH(OBJECT, POP2(op_range, NUMBER, NUMBER));
                break;
            case KALOS_OP_MAX:
                internal_error(state); // impossible
                break;
        }
    }
}

void kalos_run_free(kalos_state state_) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    state->fns->free(state);
}

kalos_object* kalos_allocate_object(kalos_state state_, size_t context_size) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    kalos_object* object = (kalos_object*)state->fns->alloc(sizeof(kalos_object) + context_size);
    memset(object, 0, sizeof(*object) + context_size);
    if (context_size) {
        object->context = (uint8_t*)object + sizeof(kalos_object);
    }
    return object;
}

void* kaloc_mem_alloc(kalos_state state_, size_t size) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    return state->fns->alloc(size);
}
