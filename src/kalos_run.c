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

// https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms
#define KM__CHECK_N(x, n, ...) n
#define KM__CHECK(...) KM__CHECK_N(__VA_ARGS__, 0,)
#define KM__PROBE(x) x, 1,
#define KM__IS_PAREN(x) KM__CHECK(KM__IS_PAREN_PROBE x)
#define KM__IS_PAREN_PROBE(...) KM__PROBE(~)
#define KM__PRIMITIVE_COMPARE(x, y) KM__IS_PAREN ( KM__COMPARE_ ## x ( KM__COMPARE_ ## y) (()) )
#define KM__COMPARE_(x)
#define KM__COMPARE_0(x)

// Magic
#define KALOS_EVAL__(...) __VA_ARGS__
#define KALOS_CONCAT_P__(a, ...) a ## __VA_ARGS__
#define KALOS_CONCAT__(a, ...) KALOS_CONCAT_P__(a, __VA_ARGS__)
#define KALOS_COUNTER__(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77, _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96, _97, _98, _99, N, ...) N
#define KALOS_COUNT__(...) KALOS_COUNTER__(__VA_ARGS__, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define KALOS_FOREACH__(X, ...) KALOS_FOREACH_N__(X, __VA_ARGS__,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,)
#define KALOS_FOREACH_N__(X, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77, _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96, _97, _98, _99, ...) \
    X(0,_0) X(1,_1) X(2,_2) X(3,_3) X(4,_4) X(5,_5) X(6,_6) X(7,_7) X(8,_8) X(9,_9) X(10,_10) X(11,_11) X(12,_12) X(13,_13) X(14,_14) X(15,_15) X(16,_16) X(17,_17) X(18,_18) X(19,_19) X(20,_20) X(21,_21) X(22,_22) X(23,_23) X(24,_24) X(25,_25) X(26,_26) X(27,_27) X(28,_28) X(29,_29) X(30,_30) X(31,_31) X(32,_32) X(33,_33) X(34,_34) X(35,_35) X(36,_36) X(37,_37) X(38,_38) X(39,_39) X(40,_40) X(41,_41) X(42,_42) X(43,_43) X(44,_44) X(45,_45) X(46,_46) X(47,_47) X(48,_48) X(49,_49) X(50,_50) X(51,_51) X(52,_52) X(53,_53) X(54,_54) X(55,_55) X(56,_56) X(57,_57) X(58,_58) X(59,_59) X(60,_60) X(61,_61) X(62,_62) X(63,_63) X(64,_64) X(65,_65) X(66,_66) X(67,_67) X(68,_68) X(69,_69) X(70,_70) X(71,_71) X(72,_72) X(73,_73) X(74,_74) X(75,_75) X(76,_76) X(77,_77) X(78,_78) X(79,_79) X(80,_80) X(81,_81) X(82,_82) X(83,_83) X(84,_84) X(85,_85) X(86,_86) X(87,_87) X(88,_88) X(89,_89) X(90,_90) X(91,_91) X(92,_92) X(93,_93) X(94,_94) X(95,_95) X(96,_96) X(97,_97) X(98,_98) X(99,_99)
#define KALOS_FOREACH2__(X, ...) KALOS_FOREACH2_N__(X, __VA_ARGS__,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,)
#define KALOS_FOREACH2_N__(X, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77, _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96, _97, _98, _99, ...) \
    X(0,_0) X(1,_1) X(2,_2) X(3,_3) X(4,_4) X(5,_5) X(6,_6) X(7,_7) X(8,_8) X(9,_9) X(10,_10) X(11,_11) X(12,_12) X(13,_13) X(14,_14) X(15,_15) X(16,_16) X(17,_17) X(18,_18) X(19,_19) X(20,_20) X(21,_21) X(22,_22) X(23,_23) X(24,_24) X(25,_25) X(26,_26) X(27,_27) X(28,_28) X(29,_29) X(30,_30) X(31,_31) X(32,_32) X(33,_33) X(34,_34) X(35,_35) X(36,_36) X(37,_37) X(38,_38) X(39,_39) X(40,_40) X(41,_41) X(42,_42) X(43,_43) X(44,_44) X(45,_45) X(46,_46) X(47,_47) X(48,_48) X(49,_49) X(50,_50) X(51,_51) X(52,_52) X(53,_53) X(54,_54) X(55,_55) X(56,_56) X(57,_57) X(58,_58) X(59,_59) X(60,_60) X(61,_61) X(62,_62) X(63,_63) X(64,_64) X(65,_65) X(66,_66) X(67,_67) X(68,_68) X(69,_69) X(70,_70) X(71,_71) X(72,_72) X(73,_73) X(74,_74) X(75,_75) X(76,_76) X(77,_77) X(78,_78) X(79,_79) X(80,_80) X(81,_81) X(82,_82) X(83,_83) X(84,_84) X(85,_85) X(86,_86) X(87,_87) X(88,_88) X(89,_89) X(90,_90) X(91,_91) X(92,_92) X(93,_93) X(94,_94) X(95,_95) X(96,_96) X(97,_97) X(98,_98) X(99,_99)

#define KALOS_VAR_SLOT_SIZE 16

static const int8_t kalos_op_input_size[] = {
#define KALOS_OP(x, in, out) in, 
#include "kalos_constants.inc"
};
static const int8_t kalos_op_output_size[] = {
#define KALOS_OP(x, in, out) out, 
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
}

void type_error(kalos_state_internal* state) {
    ASSERT(false);
}

void value_error(kalos_state_internal* state) {
    ASSERT(false);
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

static kalos_value sized_iternext(kalos_state state, kalos_object* iter, bool* done) {
    struct sized_iterator_context* sized_context = iter->context;
    kalos_value value = {0};
    if (sized_context->index >= sized_context->count) {
        *done = true;
        return value;
    }
    *done = false;
    sized_context->fn(state, (uint8_t*)iter->context + sizeof(struct sized_iterator_context), sized_context->index++, &value);
    return value;
}

static kalos_object* sized_iterstart(kalos_state state, kalos_object* range) {
    struct sized_iterator_context* sized_context = range->context;
    kalos_object* iter = kalos_allocate_object(state, sized_context->context_size);
    memcpy(iter->context, sized_context, sized_context->context_size);
    iter->iternext = sized_iternext;
    return iter;
}

kalos_object* kalos_allocate_sized_iterable(kalos_state state, kalos_iterable_fn fn, size_t context_size, void** context, uint16_t count) {
    // This currently copies the entire context to each iterator which is not as efficient as just
    // retaining the parent, but that requires a bit more complex code and might be slower overall
    kalos_object* obj = kalos_allocate_object(state, sizeof(struct sized_iterator_context) + context_size);
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

static kalos_object* op_range(kalos_state_internal* state, kalos_op op, kalos_int start, kalos_int end) {
    kalos_int* range_spec;
    kalos_object* range = kalos_allocate_sized_iterable(state, range_iterfunc, sizeof(kalos_int[2]), (void**)&range_spec, end <= start ? 0 : end - start);
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

static void split_free(kalos_state state, kalos_object* object) {
    struct kalos_split* split_context = (struct kalos_split*)object->context;
    kalos_string_release(state, split_context->splitee);
    kalos_string_release(state, split_context->splitter);
}

static kalos_value split_iternext(kalos_state state, kalos_object* iter, bool* done) {
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
        kalos_string splitee = kalos_string_duplicate(state, split_context->splitee);
        value.value.string = kalos_string_take_substring(state, &splitee, split_context->offset, found_offset - split_context->offset);
        split_context->offset += (found_offset - split_context->offset) + split_context->splitter_length;
    }
    VALIDATE_STRING(value.value.string);
    return value;
}

static kalos_object* split_iterstart(kalos_state state, kalos_object* split) {
    struct kalos_split* split_context = (struct kalos_split*)split->context;
    kalos_object* iter = kalos_allocate_object(state, sizeof(struct kalos_split));
    struct kalos_split* iter_context = (struct kalos_split*)iter->context;
    *iter_context = *split_context;
    split_context->splitee = kalos_string_duplicate(state, split_context->splitee);
    split_context->splitter = kalos_string_duplicate(state, split_context->splitter);
    iter->iternext = split_iternext;
    iter->object_free = split_free;
    return iter;
}

static kalos_object* op_split(kalos_state_internal* state, kalos_op op, kalos_string* splitee, kalos_string* splitter) {
    kalos_object* split = kalos_allocate_object(state, sizeof(struct kalos_split));
    struct kalos_split* split_context = (struct kalos_split*)split->context;
    split_context->length = kalos_string_length(state, *splitee);
    split_context->splitee = kalos_string_take(state, splitee);
    split_context->splitter_length = kalos_string_length(state, *splitter);
    split_context->splitter = kalos_string_take(state, splitter);
    split->iterstart = split_iterstart;
    split->object_free = split_free;
    return split;
}

static kalos_object* op_iterator(kalos_state_internal* state, kalos_op op, kalos_object* iterable) {
    return iterable->iterstart(state, iterable);
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

void kalos_load_arg_object(kalos_state state_, kalos_int index, kalos_object* arg) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    state->stack.stack[state->stack.stack_index + index].type = KALOS_VALUE_OBJECT;
    state->stack.stack[state->stack.stack_index + index].value.object = arg;
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
                kalos_value next = iterator->value.object->iternext(state_, iterator->value.object, &done);
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
                int count = kalos_stack_fixup_varargs(0, &state->stack);
                kalos_object* list = kalos_allocate_list(state, count, peek(&state->stack, -1));
                kalos_stack_cleanup_varargs(0, count, state, &state->stack);
                push_object(&state->stack, list);
                break;
            }
            case KALOS_OP_GETPROP: {
                int prop = pop(&state->stack)->value.number;
                kalos_object* object = pop(&state->stack)->value.object;
                if (!object->dispatch || !object->dispatch(state, object, prop, &state->stack)) {
                    value_error(state);
                }
                kalos_object_release(state, object);
                break;
            }
            case KALOS_OP_SETPROP: {
                int prop = pop(&state->stack)->value.number;
                kalos_value* value = pop(&state->stack);
                kalos_object* object = pop(&state->stack)->value.object;
                *push_raw(&state->stack) = *value;
                if (!object->dispatch || !object->dispatch(state, object, prop, &state->stack)) {
                    value_error(state);
                }
                kalos_object_release(state, object);
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

#define arg_OBJECT(x) , state->stack.stack[state->stack.stack_index+x].value.object
#define save_OBJECT kalos_object* o = 
#define push_OBJECT state->stack.stack[state->stack.stack_index].type = KALOS_VALUE_OBJECT; state->stack.stack[state->stack.stack_index++].value.object = o;
#define cleanup_OBJECT(x) kalos_object_release(state, state->stack.stack[state->stack.stack_index+x].value.object);

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

        // printf("stack %s:", kalos_op_strings[op]);
        // for (int i = 0; i < state->stack.stack_index; i++) {
        //     printf(" %d=", i);
        //     switch (state->stack.stack[i].type) {
        //         case KALOS_VALUE_NONE:
        //             printf("(none)");
        //             break;
        //         case KALOS_VALUE_BOOL:
        //             printf("(bool)%d", state->stack.stack[i].value.number);
        //             break;
        //         case KALOS_VALUE_NUMBER:
        //             printf("%d", state->stack.stack[i].value.number);
        //             break;
        //         case KALOS_VALUE_STRING:
        //             printf("\"%s\"", kalos_string_c(state, state->stack.stack[i].value.string));
        //             break;
        //         case KALOS_VALUE_OBJECT:
        //             printf("(object)");
        //             break;
        //     }
        // }
        // printf("\n");

        // Confirm stack is correct
        if (kalos_op_input_size[op] != -1 && kalos_op_output_size[op] != -1) {
            int diff = state->stack.stack_index - stack_index;
            ASSERT(diff == kalos_op_output_size[op] - kalos_op_input_size[op]);
        }
    }

    done:
    for (int i = original_stack_index; i < state->stack.stack_index; i++) {
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
    kalos_object* object;
    kalos_int index;
};

kalos_value list_iternext(kalos_state state, kalos_object* object, bool* done) {
    struct list_iter_context* context = object->context;
    kalos_value* array = context->object->context;
    if (context->index >= array[0].value.number) {
        *done = true;
        kalos_object_release(state, context->object);
        kalos_value none = {0};
        return none;
    }
    *done = false;
    return kalos_value_clone(state, &array[1 + context->index++]);
}

kalos_object* list_iterstart(kalos_state state, kalos_object* object) {
    kalos_object* iter = kalos_allocate_object(state, sizeof(struct list_iter_context));
    struct list_iter_context* context = iter->context;
    kalos_object_retain(state, object);
    context->index = 0;
    context->object = object;
    iter->iternext = list_iternext;
    return iter;
}

void list_free(kalos_state state_, kalos_object* object) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    kalos_value* array = object->context;
    for (int i = 0; i < array[0].value.number; i++) {
        kalos_clear(state, &array[i + 1]);
    }
    state->fns->free(array);
}

kalos_object* kalos_allocate_list(kalos_state state_, kalos_int size, kalos_value* values) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    kalos_object* object = kalos_allocate_object(state, 0);
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

kalos_object* kalos_allocate_object(kalos_state state_, size_t context_size) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    kalos_object* object = (kalos_object*)state->fns->alloc(sizeof(kalos_object) + context_size);
    memset(object, 0, sizeof(*object) + context_size);
    if (context_size) {
        object->context = (uint8_t*)object + sizeof(kalos_object);
    }
    return object;
}

kalos_object* kalos_allocate_prop_object(kalos_state state, void* context, kalos_object_dispatch dispatch) {
    kalos_object* object = kalos_allocate_object(state, 0);
    object->context = context;
    object->dispatch = dispatch;
    return object;
}

void* kaloc_mem_alloc(kalos_state state_, size_t size) {
    kalos_state_internal* state = (kalos_state_internal*)state_;
    return state->fns->alloc(size);
}
