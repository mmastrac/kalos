#pragma once

#include "kalos.h"
#include "_kalos_string_system.h"

typedef struct kalos_object kalos_object;
typedef kalos_object* kalos_object_ref;
typedef struct kalos_value kalos_value;
typedef struct kalos_stack kalos_stack;
typedef struct kalos_run_state kalos_run_state;
typedef kalos_string (*kalos_tostring)(kalos_state* state, kalos_object_ref* object);
typedef kalos_value (*kalos_getindex)(kalos_state* state, kalos_object_ref* object, kalos_int index);
typedef kalos_int (*kalos_getlength)(kalos_state* state, kalos_object_ref* object);
typedef kalos_object_ref (*kalos_iterstart)(kalos_state* state, kalos_object_ref* object);
typedef kalos_value (*kalos_iternext)(kalos_state* state, kalos_object_ref* object, bool* done);
typedef void (*kalos_object_free)(kalos_state* state, kalos_object_ref* object);

typedef bool (*kalos_dispatch_name_fn)(kalos_run_state* state, const char* module, const char* name, int param_count, kalos_stack* stack, bool retval);
typedef bool (*kalos_dispatch_fn)(kalos_run_state* state, int function, int param_count, kalos_stack* stack, bool retval);
typedef void (*kalos_idl_dispatch_fn)(kalos_run_state* state, kalos_value* idl);
typedef bool (*kalos_object_dispatch_name_fn)(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack);
typedef bool (*kalos_object_dispatch_fn)(kalos_run_state* state, kalos_object_ref* object, int function, int param_count, kalos_stack* stack);

typedef struct kalos_dispatch {
    kalos_dispatch_name_fn dispatch_name;
    kalos_dispatch_fn* modules;
    kalos_idl_dispatch_fn idl;
} kalos_dispatch;

typedef struct kalos_object_dispatch {
    kalos_object_dispatch_name_fn dispatch_name;
    kalos_object_dispatch_fn dispatch_id;
} kalos_object_dispatch;

typedef enum kalos_value_type {
    KALOS_VALUE_NONE,
    KALOS_VALUE_BOOL,
    KALOS_VALUE_NUMBER,
    KALOS_VALUE_STRING,
    KALOS_VALUE_OBJECT,
} kalos_value_type;

struct kalos_object {
    void* context;
    uint16_t count;
    kalos_object_free object_free;
    kalos_getindex getindex;
    kalos_getlength getlength;
    kalos_tostring tostring;
    kalos_iterstart iterstart;
    kalos_iternext iternext;
    kalos_object_dispatch* dispatch;
};

typedef union kalos_value_union {
    kalos_int number;
    kalos_string string;
    kalos_object_ref object;
} kalos_value_union;

struct kalos_value {
    kalos_value_type type;
    kalos_value_union value;
};

#define KALOS_STACK_SIZE 64

typedef void (*stack_error)(const char* message);

typedef struct kalos_stack {
    kalos_value stack[KALOS_STACK_SIZE];
    int stack_index;
    stack_error stack_error;
} kalos_stack;

#define KALOS_RUN_ENVIRONMENT \
    KALOS_BASIC_ENVIRONMENT;                 \
    /**                                      \
     * Allows access to the runtime stack.   \
     */                                      \
    kalos_stack* stack                       \

typedef struct kalos_run_state {
    KALOS_RUN_ENVIRONMENT;
} kalos_run_state;

static inline kalos_state* kalos_state_from_run_state(kalos_run_state* run_state) { return (kalos_state*)run_state; }

#define KALOS_OBJECT_POISONED 0x7fff

static inline void kalos_clear(kalos_state* state, kalos_value* value);

static inline kalos_object_ref kalos_object_take(kalos_state* state, kalos_object_ref* object) {
    kalos_object_ref obj = *object;
    *object = NULL;
    return obj;
}

static void kalos_object_retain(kalos_state* state, kalos_object_ref object) {
    object->count++;
}

void kalos_object_release(kalos_state* state, kalos_object_ref* ref);

inline static int kalos_stack_vararg_count(kalos_stack* stack) {
    return stack->stack[stack->stack_index - 1].value.number;
}

inline static kalos_value* kalos_stack_vararg_start(kalos_stack* stack, int vararg_count) {
    return &stack->stack[stack->stack_index - vararg_count - 1];
}

#define KALOS_VALUE_ANY (999)
#define KALOS_CHECK_STACK_REF(N) state->stack->stack[state->stack->stack_index+N]
#define KALOS_CHECK_1__                    (type0 == KALOS_VALUE_ANY || KALOS_CHECK_STACK_REF(0).type == type0)
#define KALOS_CHECK_2__ KALOS_CHECK_1__ && (type1 == KALOS_VALUE_ANY || KALOS_CHECK_STACK_REF(1).type == type1)
#define KALOS_CHECK_3__ KALOS_CHECK_2__ && (type2 == KALOS_VALUE_ANY || KALOS_CHECK_STACK_REF(2).type == type2)
#define KALOS_CHECK_4__ KALOS_CHECK_3__ && (type3 == KALOS_VALUE_ANY || KALOS_CHECK_STACK_REF(3).type == type3)
#define KALOS_CHECK_5__ KALOS_CHECK_4__ && (type4 == KALOS_VALUE_ANY || KALOS_CHECK_STACK_REF(4).type == type4)
#define KALOS_CHECK_6__ KALOS_CHECK_5__ && (type5 == KALOS_VALUE_ANY || KALOS_CHECK_STACK_REF(5).type == type5)
#define KALOS_CHECK_7__ KALOS_CHECK_6__ && (type6 == KALOS_VALUE_ANY || KALOS_CHECK_STACK_REF(6).type == type6)
#define KALOS_CHECK_8__ KALOS_CHECK_7__ && (type7 == KALOS_VALUE_ANY || KALOS_CHECK_STACK_REF(7).type == type7)
#define KALOS_CHECK__(N) return KALOS_CHECK_##N##__

inline static bool kalos_stack_setup_0(kalos_run_state* state) { return true; }
inline static bool kalos_stack_setup_1(kalos_run_state* state, kalos_value_type type0) { KALOS_CHECK__(1); }
inline static bool kalos_stack_setup_2(kalos_run_state* state, kalos_value_type type0, kalos_value_type type1) { KALOS_CHECK__(2); }
bool kalos_stack_setup_3(kalos_run_state* state, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2);
bool kalos_stack_setup_4(kalos_run_state* state, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3);
bool kalos_stack_setup_5(kalos_run_state* state, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4);
bool kalos_stack_setup_6(kalos_run_state* state, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5);
bool kalos_stack_setup_7(kalos_run_state* state, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5, kalos_value_type type6);
bool kalos_stack_setup_8(kalos_run_state* state, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5, kalos_value_type type6, kalos_value_type type7);

static inline void kalos_stack_cleanup(kalos_run_state* state, int count) {
    state->stack->stack_index -= count;
    for (int i = 0; i < count; i++) {
        kalos_clear(kalos_state_from_run_state(state), &state->stack->stack[state->stack->stack_index + i]);
    }
}

// DANGEROUS: Releases a value but doesn't set its type
static inline void kalos_release__(kalos_state* state, kalos_value* value) {
    if (value->type == KALOS_VALUE_STRING) {
        kalos_string_release(state, value->value.string);
    } else if (value->type == KALOS_VALUE_OBJECT) {
        kalos_object_release(state, &value->value.object);
    }
}

// DANGEROUS: Retains a value
static inline void kalos_retain__(kalos_state* state, kalos_value* value) {
    if (value->type == KALOS_VALUE_STRING) {
        value->value.string = kalos_string_duplicate(state, value->value.string);
    } if (value->type == KALOS_VALUE_OBJECT) {
        kalos_object_retain(state, value->value.object);
    }
}

// Clears a value and sets its type to NONE
static inline void kalos_clear(kalos_state* state, kalos_value* value) {
    kalos_release__(state, value);
    value->type = KALOS_VALUE_NONE;
}

// Moves a value from one location to another, clearing the old value
static inline kalos_value kalos_value_move(kalos_state* state, kalos_value* value) {
    kalos_value v = *value;
    value->type = KALOS_VALUE_NONE;
    return v;
}

// Moves a value from one location to another, clearing the old value
static inline void kalos_value_move_to(kalos_state* state, kalos_value* from, kalos_value* to) {
    kalos_release__(state, to);
    *to = *from;
    from->type = KALOS_VALUE_NONE;
}

// Copies a value from one location to another, retaining if necessary
static inline kalos_value kalos_value_clone(kalos_state* state, kalos_value* value) {
    kalos_value v = *value;
    kalos_retain__(state, &v);
    return v;
}

// Copies a value from one location to another, retaining if necessary
static inline void kalos_value_clone_to(kalos_state* state, kalos_value* from, kalos_value* to) {
    kalos_clear(state, to);
    *to = *from;
    kalos_retain__(state, to);
}

#define KALOS_VALUE(TYPE, name_, field_, type_, ptr_, take_) \
static inline void push_##name_(kalos_stack* stack, type_ value) {\
    stack->stack[stack->stack_index].type = KALOS_VALUE_##TYPE; \
    stack->stack[stack->stack_index].value.field_ = value; \
    stack->stack_index++; \
} \
static inline void kalos_load_arg_##name_(kalos_run_state* state, kalos_int index, type_ ptr_ arg) {\
    state->stack->stack[state->stack->stack_index + index].type = KALOS_VALUE_##TYPE; \
    state->stack->stack[state->stack->stack_index + index].value.field_ = take_; \
}

KALOS_VALUE(NUMBER, number, number, kalos_int,,         arg)
KALOS_VALUE(BOOL,   bool,   number, kalos_int,,         arg)
KALOS_VALUE(STRING, string, string, kalos_string,*,     kalos_string_take(kalos_state_from_run_state(state), arg))
KALOS_VALUE(OBJECT, object, object, kalos_object_ref,*, kalos_object_take(kalos_state_from_run_state(state), arg))

static inline kalos_value* pop(kalos_stack* stack) {
    return &stack->stack[--stack->stack_index];
}

static inline kalos_value* peek(kalos_stack* stack, int offset) {
    int stack_offset = stack->stack_index - 1 - offset;
    if (stack_offset < 0) {
        stack->stack_error("peek below stack");
        return &stack->stack[0];
    }
    return &stack->stack[stack_offset];
}

static inline kalos_value* push_raw(kalos_stack* stack) {
    return &stack->stack[stack->stack_index++];
}

static inline void kalos_load_arg_any(kalos_run_state* state, kalos_int index, kalos_value* arg) {
    kalos_value_move_to(kalos_state_from_run_state(state), arg, &state->stack->stack[state->stack->stack_index + index]);
}

bool kalos_coerce(kalos_state* state, kalos_value* v, kalos_value_type type);
kalos_object_ref kalos_allocate_object(kalos_state* state, size_t context_size);
kalos_object_ref kalos_allocate_list(kalos_state* state, kalos_int size, kalos_value* values);
typedef void (*kalos_iterable_fn)(kalos_state* state, void* context, uint16_t index, kalos_value* output);
kalos_object_ref kalos_allocate_sized_iterable(kalos_state* state, kalos_iterable_fn fn, size_t context_size, void** context, uint16_t count);
kalos_object_ref kalos_allocate_prop_object(kalos_state* state, void* context, kalos_object_dispatch* dispatch);
