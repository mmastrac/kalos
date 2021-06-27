#include "_kalos_defines.h"
#include "_kalos_string_format.h"
#include "_kalos_string_system.h"
#include "kalos_run.h"

#define KALOS_VAR_SLOT_SIZE 16
#define PC_DONE 0xffff

typedef struct kalos_state_internal {
    KALOS_RUN_ENVIRONMENT;
    const_kalos_script script;
    kalos_dispatch* dispatch;
    kalos_value globals[KALOS_VAR_SLOT_SIZE];
    kalos_value* locals;
    uint16_t pc;
} kalos_state_internal;

void kalos_internal_error(kalos_state* state) {
    ASSERT(false);
    LOG("internal error");
    state->error(0, "Internal error");
}

void kalos_type_error(kalos_state* state) {
    ASSERT(false);
    LOG("type error");
    state->error(0, "Type error");
}

void kalos_value_error(kalos_state* state) {
    ASSERT(false);
    LOG("value error");
    state->error(0, "Value error");
}

#define ENSURE_STACK(size) { if (state->stack->stack_index < size) { kalos_internal_error((kalos_state*)state); } }
#define ENSURE_STACK_SPACE(size) { if (KALOS_STACK_SIZE - state->stack->stack_index - 1 < size) { kalos_internal_error((kalos_state*)state); } }

static kalos_int read_inline_integer(kalos_state_internal* state) {
    kalos_int int_value;
    script_memcpy(&int_value, &state->script[state->pc], sizeof(int_value));
    state->pc += sizeof(kalos_int);
    return int_value;
}

static kalos_value op_logical_op(kalos_state* state, kalos_op op, kalos_value* a, kalos_value* b) {
    kalos_value v = kalos_value_clone(state, a);
    kalos_coerce(state, &v, KALOS_VALUE_BOOL);
    if (v.value.number ^ (op == KALOS_OP_LOGICAL_AND)) {
        return kalos_value_move(state, a);
    } else {
        return kalos_value_move(state, b);
    }
}

static kalos_int op_compare_string(kalos_state* state, kalos_op op, kalos_string* a, kalos_string* b) {
    kalos_int compare = kalos_string_compare(state, *a, *b);
    switch (op) {
        case KALOS_OP_EQUAL: return compare == 0;
        case KALOS_OP_NOT_EQUAL: return compare != 0;
        case KALOS_OP_GT: return compare > 0;
        case KALOS_OP_GTE: return compare >= 0;
        case KALOS_OP_LT: return compare < 0;
        case KALOS_OP_LTE: return compare <= 0;
        default: kalos_internal_error(state); return 0;
    }
}

static kalos_int op_compare_type(kalos_state* state, kalos_op op, kalos_value* a, kalos_value* b) {
    if (op == KALOS_OP_EQUAL) {
        return a->type == b->type;
    } else if (op == KALOS_OP_NOT_EQUAL) {
        return a->type != b->type;
    }
    return 0;
}

static kalos_string op_string_add(kalos_state* state, kalos_op op, kalos_string* a, kalos_value* b) {
    kalos_coerce(state, b, KALOS_VALUE_STRING);
    return kalos_string_take_append(state, a, &b->value.string);
}

static kalos_string op_string_add2(kalos_state* state, kalos_op op, kalos_value* a, kalos_string* b) {
    kalos_coerce(state, a, KALOS_VALUE_STRING);
    return kalos_string_take_append(state, &a->value.string, b);
}

static kalos_string op_string_multiply(kalos_state* state, kalos_op op, kalos_string* a, kalos_int b) {
    if (b <= 0 || kalos_string_isempty(state, *a)) {
        return kalos_string_allocate(state, "");
    }
    return kalos_string_take_repeat(state, a, b);
}

static kalos_string op_string_multiply2(kalos_state* state, kalos_op op, kalos_int a, kalos_string* b) {
    return op_string_multiply(state, op, b, a);
}

static kalos_int op_string_number(kalos_state* state, kalos_op op, kalos_string* v) {
    switch (op) {
        case KALOS_OP_ORD: return kalos_string_char_at(state, *v, 0);
        case KALOS_OP_LENGTH: return kalos_string_length(state, *v);
        default: return 0;
    }
}

static kalos_string op_string_getindex(kalos_state* state, kalos_op op, kalos_string* s, kalos_int index) {
    if (index < 0 || index >= kalos_string_length(state, *s)) {
        return kalos_string_allocate(state, "");
    }
    kalos_writable_string str = kalos_string_allocate_writable_size(state, 1);
    char* out = kalos_string_writable_c(state, str);
    out[0] = kalos_string_c(state, *s)[index];
    return kalos_string_commit(state, str);
}

static kalos_string op_to_hex_or_char(kalos_state* state, kalos_op op, kalos_int v) {
    if (op == KALOS_OP_TO_HEX) {
        return kalos_string_allocate_fmt(state, "0x%x", v);
    } else if (op == KALOS_OP_TO_CHAR) {
        kalos_writable_string str = kalos_string_allocate_writable_size(state, 1);
        char* s = kalos_string_writable_c(state, str);
        s[0] = v;
        return kalos_string_commit(state, str);
    }
    kalos_internal_error(state);
    return kalos_string_allocate(state, "");
}

static kalos_string op_to_string(kalos_state* state, kalos_op op, kalos_value* v) {
    kalos_coerce(state, v, KALOS_VALUE_STRING);
    kalos_string s = kalos_string_take(state, &v->value.string);
    return s;
}

static kalos_int op_to_bool(kalos_state* state, kalos_op op, kalos_value* v) {
    kalos_coerce(state, v, KALOS_VALUE_BOOL);
    return v->value.number ^ (op == KALOS_OP_LOGICAL_NOT);
}

static kalos_int op_to_int(kalos_state* state, kalos_op op, kalos_value* v) {
    kalos_coerce(state, v, KALOS_VALUE_NUMBER);
    return v->value.number;
}

static void op_goto(kalos_state_internal* state, kalos_op op) {
    state->pc = read_inline_integer(state);
}

static void op_goto_if(kalos_state_internal* state, kalos_op op, kalos_int flag) {
    kalos_int pc = read_inline_integer(state);
    if (flag) {
        state->pc = pc;
    }
}

void kalos_trigger_pc(kalos_run_state* state_, kalos_int pc, const kalos_section_header kalos_far* header, bool keep_ret);

static void op_gosub(kalos_state_internal* state, kalos_op op, kalos_int param_count, kalos_value* v) {
    kalos_int pc = read_inline_integer(state);
    // Save and restore the stack to where the subroutine expects it to be
    state->stack->stack_index -= param_count + 1;
    const kalos_section_header kalos_far* header = (const kalos_section_header kalos_far*)&state->script[pc];
    pc += sizeof(kalos_section_header);
    kalos_trigger_pc((kalos_run_state*)state, pc, header, op == KALOS_OP_GOSUB);
    state->stack->stack_index += param_count + 1;
}

struct kalos_range {
    kalos_int current;
    kalos_int start;
    kalos_int end;
    kalos_int step;
};

static void range_iterfunc(kalos_state* state, void* context, uint16_t index, kalos_value* value) {
    kalos_int* range_spec = context;
    value->type = KALOS_VALUE_NUMBER;
    value->value.number = range_spec[0] + range_spec[1] * index;
}

static kalos_object_ref op_range(kalos_state* state, kalos_op op, kalos_int start, kalos_int end) {
    kalos_int* range_spec;
    kalos_object_ref range = kalos_allocate_sized_iterable(state, range_iterfunc, sizeof(kalos_int[2]), (void**)&range_spec, end <= start ? 0 : end - start);
    range_spec[0] = start;
    range_spec[1] = 1; // step (TODO)
    return range;
}

static kalos_string op_replace(kalos_state* state, kalos_op op, kalos_string* s, kalos_string* a, kalos_string* b) {
    return kalos_string_take_replace(state, s, a, b);
}

struct kalos_split {
    kalos_string splitee;
    kalos_string splitter;
    int offset;
    int length;
    int splitter_length;
};

static void split_free(kalos_state* state, kalos_object_ref* object) {
    struct kalos_split* split_context = (struct kalos_split*)(*object)->context;
    kalos_string_release(state, split_context->splitee);
    kalos_string_release(state, split_context->splitter);
}

static kalos_value split_iternext(kalos_state* state, kalos_object_ref* iter, bool* done) {
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

static kalos_object_ref split_iterstart(kalos_state* state, kalos_object_ref* split) {
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

static kalos_object_ref op_split(kalos_state* state, kalos_op op, kalos_string* splitee, kalos_string* splitter) {
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

static kalos_object_ref op_iterator(kalos_state* state, kalos_op op, kalos_object_ref* iterable) {
    return (*iterable)->iterstart(state, iterable);
}

static kalos_string op_push_string(kalos_state_internal* internal_state, kalos_op op) {
    kalos_state* state = (kalos_state*)internal_state;
    kalos_string string = kalos_string_allocate(state, (const char*)&internal_state->script[internal_state->pc]);
    LOG("push: %s", kalos_string_c(state, string));
    internal_state->pc += kalos_string_length(state, string) + 1;
    return string;
}

static kalos_int op_push_integer(kalos_state_internal* state, kalos_op op) {
    return read_inline_integer(state);
}

static kalos_string op_format(kalos_state_internal* internal_state, kalos_op op, kalos_value* v) {
    kalos_state* state = (kalos_state*)internal_state;
    kalos_string_format* format = (kalos_string_format*)&internal_state->script[internal_state->pc];
    internal_state->pc += sizeof(kalos_string_format);
    if (v->type == KALOS_VALUE_NUMBER) {
        return kalos_string_format_int(state, v->value.number, format);
    } else if (v->type == KALOS_VALUE_STRING) {
        int size = snprintf(NULL, 0, "%*.*s",
            format->align == KALOS_STRING_FORMAT_ALIGN_LEFT ? -format->min_width : format->min_width,
            format->precision ? format->precision : 0xffff,
            kalos_string_c(state, v->value.string));
        kalos_writable_string str = kalos_string_allocate_writable_size(state, size);
        char* s = kalos_string_writable_c(state, str);
        sprintf(s, "%*.*s",
            format->align == KALOS_STRING_FORMAT_ALIGN_LEFT ? -format->min_width : format->min_width,
            format->precision ? format->precision : 0xffff,
            kalos_string_c(state, v->value.string));
        return kalos_string_commit(state, str);
    }
    return kalos_string_allocate(state, "");
}

static void op_store(kalos_state_internal* state, kalos_op op, kalos_value* v, kalos_int slot) {
    kalos_value* storage = op == KALOS_OP_STORE_LOCAL ? state->locals : state->globals;
    kalos_value_move_to((kalos_state*)state, v, &storage[slot]);
}

static kalos_value op_load(kalos_state_internal* state, kalos_op op, kalos_int slot) {
    kalos_value* storage = op == KALOS_OP_LOAD_LOCAL ? state->locals : state->globals;
    return kalos_value_clone((kalos_state*)state, &storage[slot]);
}

static kalos_object_ref op_make_list(kalos_state_internal* state, kalos_op op, kalos_int count, kalos_value* v) {
    return kalos_allocate_list((kalos_state*)state, count, v);
}

static void op_idl(kalos_state_internal* state, kalos_op op, kalos_object_ref* idl) {
    kalos_string s = (*idl)->tostring((kalos_state*)state, idl);
    LOG("%s", kalos_string_c((kalos_state*)state, s));
    kalos_string_release((kalos_state*)state, s);
    if (!state->dispatch->idl) {
        kalos_value_error((kalos_state*)state);
        return;
    }
    kalos_value v;
    v.type = KALOS_VALUE_OBJECT;
    v.value.object = *idl;
    state->dispatch->idl((kalos_run_state*)state, &v);
}

static kalos_value op_getindex(kalos_run_state* state, kalos_op op, kalos_object_ref* object_, kalos_int index) {
    kalos_object_ref object = *object_;
    if (!object->getindex) {
        kalos_value_error((kalos_state*)state);
    }
    return object->getindex((kalos_state*)state, &object, index);
}

static void op_end(kalos_state_internal* state, kalos_op op) {
    state->pc = PC_DONE;
}

kalos_run_state* kalos_init(const_kalos_script script, kalos_dispatch* dispatch, kalos_state* state_provided) {
    kalos_state_internal* state = state_provided->alloc(sizeof(kalos_state_internal));
    if (!state) {
        if (state_provided->error) {
            state_provided->error(0, "malloc");
        }
        return NULL;
    }
    memset(state, 0, sizeof(kalos_state_internal));
    memcpy(state, state_provided, sizeof(kalos_state));
    state->stack = state_provided->alloc(sizeof(*state->stack));
    memset(state->stack, 0, sizeof(*state->stack));
    state->dispatch = dispatch;
    state->script = script;
    LOG("%s", "Triggering global");
    kalos_trigger((kalos_run_state*)state, kalos_make_address(-1, -1));
    return (kalos_run_state*)state;
}

#include "_kalos_ops.dispatch.inc"

void kalos_trigger_pc(kalos_run_state* state_, kalos_int pc, const kalos_section_header kalos_far* header, bool keep_ret) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    int original_stack_index = state->stack->stack_index;
    int original_pc = state->pc;
    state->pc = pc;
    kalos_value* original_locals = state->locals;
    state->locals = &state->stack->stack[original_stack_index];
    state->stack->stack_index += header->locals_size;
    size_t script_size = ((const kalos_script_header kalos_far*)state->script)->length;
    while (state->pc != PC_DONE) {
        if (state->pc >= script_size) {
            state->error(0, "Internal error");
            goto done;
        }
        kalos_op op = state->script[state->pc++];
        if (op >= KALOS_OP_MAX || state->stack->stack_index < 0) {
            state->error(0, "Internal error");
            goto done;
        }
        LOG("PC %04x exec %s (stack = %d)", state->pc - 1, kalos_op_strings[op], state->stack->stack_index);
        // for (int i = 0; i < state->stack->stack_index; i++) {
        //     kalos_value v = kalos_value_clone((kalos_state*)state, &state->stack->stack[i]);
        //     const char* t;
        //     switch (v.type) {
        //         case KALOS_VALUE_BOOL: t = "bool"; break;
        //         case KALOS_VALUE_NUMBER: t = "number"; break;
        //         case KALOS_VALUE_STRING: t = "string"; break;
        //         case KALOS_VALUE_NONE: t = "none"; break;
        //         case KALOS_VALUE_OBJECT: t = "object"; break;
        //     }
        //     if (kalos_coerce((kalos_state*)state, &v, KALOS_VALUE_STRING)) {
        //         const char* s = kalos_string_c((kalos_state*)state, v.value.string);
        //         if (strlen(s) > 20) {
        //             printf("[%s:%.20s...] ", t, s);
        //         } else {
        //             printf("[%s:%s] ", t, s);
        //         }
        //     } else {
        //         printf("[%s] ", t);
        //     }
        //     kalos_clear((kalos_state*)state, &v);
        // }
        // printf("\n");
        switch (op) {
            case KALOS_OP_DEBUGGER: 
                // Set breakpoints here
                break;
            case KALOS_OP_DUP: {
                kalos_value* v = peek(state->stack, 0);
                kalos_value_clone_to((kalos_state*)state, v, push_raw(state->stack));
                break;
            }
            case KALOS_OP_ITERATOR_NEXT: {
                kalos_value* iterator = peek(state->stack, 0);
                bool done;
                if (iterator->type != KALOS_VALUE_OBJECT) {
                    kalos_value_error((kalos_state*)state);
                    return;
                }
                kalos_value next = iterator->value.object->iternext((kalos_state*)state, &iterator->value.object, &done);
                push_bool(state->stack, done);
                kalos_value_move_to((kalos_state*)state, &next, push_raw(state->stack));
                break;
            }
            case KALOS_OP_CALL_NORET:
            case KALOS_OP_CALL: {
                int export = pop(state->stack)->value.number;
                int module = pop(state->stack)->value.number;
                int count = read_inline_integer(state);
                if (!state->dispatch->modules[module](state_, export, count, state->stack, op == KALOS_OP_CALL)) {
                    LOG("Invoke failed %d %d %d", module, export, count);
                    kalos_value_error((kalos_state*)state);
                }
                break;
            }
            case KALOS_OP_CALL_BYNAME:
            case KALOS_OP_CALL_BYNAME_NORET: {
                kalos_string export = pop(state->stack)->value.string;
                kalos_string module = pop(state->stack)->value.string;
                int count = read_inline_integer(state);
                if (!state->dispatch->dispatch_name || !(*state->dispatch->dispatch_name)(state_, kalos_string_c((kalos_state*)state, module), kalos_string_c((kalos_state*)state, export), count, state->stack, op == KALOS_OP_CALL_BYNAME)) {
                    LOG("Invoke failed %s %s %d", kalos_string_c((kalos_state*)state, module), kalos_string_c((kalos_state*)state, export), count);
                    kalos_value_error((kalos_state*)state);
                }
                break;
            }
            case KALOS_OP_OBJCALL: {
                int prop = pop(state->stack)->value.number;
                int count = read_inline_integer(state);
                if (peek(state->stack, count)->type != KALOS_VALUE_OBJECT) {
                    kalos_value_error((kalos_state*)state);
                    return;
                }
                kalos_object_ref object = peek(state->stack, count)->value.object;
                if (!object->dispatch || !object->dispatch->dispatch_id || !object->dispatch->dispatch_id(state_, &object, prop, count, state->stack)) {
                    kalos_value_error((kalos_state*)state);
                }
                break;
            }
            case KALOS_OP_OBJCALL_BYNAME: {
                kalos_string prop = pop(state->stack)->value.string;
                int count = read_inline_integer(state);
                if (peek(state->stack, 0)->type != KALOS_VALUE_OBJECT) {
                    kalos_value_error((kalos_state*)state);
                    return;
                }
                kalos_object_ref object = peek(state->stack, 0)->value.object;
                if (!object->dispatch || !object->dispatch->dispatch_name || !object->dispatch->dispatch_name(state_, &object, kalos_string_c((kalos_state*)state, prop), count, state->stack)) {
                    kalos_value_error((kalos_state*)state);
                }
                break;
            }
            default:
                if (!kalos_run_dispatch_ops(state_, op, state->stack)) {
                    kalos_internal_error((kalos_state*)state); // should be impossible
                    return;
                }
                break;
        }
    }

    done: {
        kalos_value ret = {0};
        if (keep_ret) {
            ret = kalos_value_clone((kalos_state*)state, pop(state->stack));
        }
        for (int i = original_stack_index; i < state->stack->stack_index; i++) {
            kalos_clear((kalos_state*)state, &state->stack->stack[i]);
        }
        state->stack->stack_index = original_stack_index;
        if (ret.type != KALOS_VALUE_NONE) {
            *push_raw(state->stack) = ret;
        }
    }
    state->pc = original_pc;
    state->locals = original_locals;
}

void kalos_run_free(kalos_run_state* state_) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    for (int i = 0; i < KALOS_VAR_SLOT_SIZE; i++) {
        kalos_clear((kalos_state*)state, &state->globals[i]);
    }
    state->free(state->stack);
    state->free(state);
}

void kalos_trigger(kalos_run_state* state_, kalos_export_address handler_address) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    const kalos_section_header kalos_far* header;
    kalos_int pc = kalos_find_section(state->script, handler_address, &header);
    if (pc == 0) {
        return;
    }
    kalos_trigger_pc(state_, pc, header, false);
}
