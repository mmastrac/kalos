#pragma once

#include <stdbool.h>
#include "kalos.h"
#include "kalos_string_system.h"

typedef struct kalos_object kalos_object;
typedef kalos_object* kalos_object_ref;
typedef struct kalos_value kalos_value;
typedef struct kalos_stack kalos_stack;
typedef kalos_value (*kalos_propget)(kalos_state state, kalos_object_ref* object, char* property);
typedef kalos_object_ref (*kalos_iterstart)(kalos_state state, kalos_object_ref* object);
typedef kalos_value (*kalos_iternext)(kalos_state state, kalos_object_ref* object, bool* done);
typedef void (*kalos_object_free)(kalos_state state, kalos_object_ref* object);
typedef bool (*kalos_object_dispatch)(kalos_state state, kalos_object_ref* object, int function, kalos_stack* stack);

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
    kalos_propget propget;
    kalos_iterstart iterstart;
    kalos_iternext iternext;
    kalos_object_dispatch dispatch;
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

#define KALOS_STACK_SIZE 32

typedef void (*stack_error)(const char* message);

typedef struct kalos_stack {
    kalos_value stack[KALOS_STACK_SIZE];
    int stack_index;
    stack_error stack_error;
} kalos_stack;

#define KALOS_OBJECT_POISONED 0x7fff

static inline void kalos_clear(kalos_state state, kalos_value* value);

static inline kalos_object_ref kalos_object_take(kalos_state state, kalos_object_ref* object) {
    kalos_object_ref obj = *object;
    *object = NULL;
    return obj;
}

static void kalos_object_retain(kalos_state state, kalos_object_ref object) {
    object->count++;
}

static void kalos_object_release(kalos_state state, kalos_object_ref* ref) {
    if (!*ref) {
        return;
    }
    kalos_object_ref object = *ref;
    if (object->count == 0) {
        if (object->object_free) {
            object->object_free(state, &object);
            object->object_free = NULL;
        }
        object->count = KALOS_OBJECT_POISONED;
        kalos_mem_free(state, object);
    } else {
        ASSERT(object->count != KALOS_OBJECT_POISONED);
        object->count--;
    }
    *ref = NULL;
}

inline static int kalos_stack_vararg_count(kalos_stack* stack) {
    return stack->stack[stack->stack_index - 1].value.number;
}

inline static kalos_value* kalos_stack_vararg_start(kalos_stack* stack, int vararg_count) {
    return &stack->stack[stack->stack_index - vararg_count - 1];
}

#define KALOS_VALUE_ANY (999)
#define KALOS_CHECK_1__                    (type0 == KALOS_VALUE_ANY || stack->stack[stack->stack_index+0].type == type0)
#define KALOS_CHECK_2__ KALOS_CHECK_1__ && (type1 == KALOS_VALUE_ANY || stack->stack[stack->stack_index+1].type == type1)
#define KALOS_CHECK_3__ KALOS_CHECK_2__ && (type2 == KALOS_VALUE_ANY || stack->stack[stack->stack_index+2].type == type2)
#define KALOS_CHECK_4__ KALOS_CHECK_3__ && (type3 == KALOS_VALUE_ANY || stack->stack[stack->stack_index+3].type == type3)
#define KALOS_CHECK_5__ KALOS_CHECK_4__ && (type4 == KALOS_VALUE_ANY || stack->stack[stack->stack_index+4].type == type4)
#define KALOS_CHECK_6__ KALOS_CHECK_5__ && (type5 == KALOS_VALUE_ANY || stack->stack[stack->stack_index+5].type == type5)
#define KALOS_CHECK_7__ KALOS_CHECK_6__ && (type6 == KALOS_VALUE_ANY || stack->stack[stack->stack_index+6].type == type6)
#define KALOS_CHECK_8__ KALOS_CHECK_7__ && (type7 == KALOS_VALUE_ANY || stack->stack[stack->stack_index+7].type == type7)
#define KALOS_CHECK__(N) return KALOS_CHECK_##N##__

inline static bool kalos_stack_setup_0(kalos_stack* stack) { return true; }
inline static bool kalos_stack_setup_1(kalos_stack* stack, kalos_value_type type0) { KALOS_CHECK__(1); }
inline static bool kalos_stack_setup_2(kalos_stack* stack, kalos_value_type type0, kalos_value_type type1) { KALOS_CHECK__(1); }
bool kalos_stack_setup_3(kalos_stack* stack, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2);
bool kalos_stack_setup_4(kalos_stack* stack, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3);
bool kalos_stack_setup_5(kalos_stack* stack, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4);
bool kalos_stack_setup_6(kalos_stack* stack, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5);
bool kalos_stack_setup_7(kalos_stack* stack, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5, kalos_value_type type6);
bool kalos_stack_setup_8(kalos_stack* stack, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5, kalos_value_type type6, kalos_value_type type7);

static inline void kalos_stack_cleanup(int count, kalos_state state, kalos_stack* stack) {
    stack->stack_index -= count;
    for (int i = 0; i < count; i++) {
        kalos_clear(state, &stack->stack[stack->stack_index + i]);
    }
}

// DANGEROUS: Releases a value but doesn't set its type
static inline void kalos_release__(kalos_state state, kalos_value* value) {
    if (value->type == KALOS_VALUE_STRING) {
        kalos_string_release(state, value->value.string);
    } else if (value->type == KALOS_VALUE_OBJECT) {
        kalos_object_release(state, &value->value.object);
    }
}

// DANGEROUS: Retains a value
static inline void kalos_retain__(kalos_state state, kalos_value* value) {
    if (value->type == KALOS_VALUE_STRING) {
        value->value.string = kalos_string_duplicate(state, value->value.string);
    } if (value->type == KALOS_VALUE_OBJECT) {
        kalos_object_retain(state, value->value.object);
    }
}

// Clears a value and sets its type to NONE
static inline void kalos_clear(kalos_state state, kalos_value* value) {
    kalos_release__(state, value);
    value->type = KALOS_VALUE_NONE;
}

// Moves a value from one location to another, clearing the old value
static inline kalos_value kalos_value_move(kalos_state state, kalos_value* value) {
    kalos_value v = *value;
    value->type = KALOS_VALUE_NONE;
    return v;
}

// Moves a value from one location to another, clearing the old value
static inline void kalos_value_move_to(kalos_state state, kalos_value* from, kalos_value* to) {
    kalos_release__(state, to);
    *to = *from;
    from->type = KALOS_VALUE_NONE;
}

// Copies a value from one location to another, retaining if necessary
static inline kalos_value kalos_value_clone(kalos_state state, kalos_value* value) {
    kalos_value v = *value;
    kalos_retain__(state, &v);
    return v;
}

// Copies a value from one location to another, retaining if necessary
static inline void kalos_value_clone_to(kalos_state state, kalos_value* from, kalos_value* to) {
    kalos_clear(state, to);
    *to = *from;
    kalos_retain__(state, to);
}

static inline void push_number(kalos_stack* stack, kalos_int value) {
    stack->stack[stack->stack_index].type = KALOS_VALUE_NUMBER;
    stack->stack[stack->stack_index].value.number = value;
    stack->stack_index++;
}

static inline void push_string(kalos_stack* stack, kalos_string value) {
    stack->stack[stack->stack_index].type = KALOS_VALUE_STRING;
    stack->stack[stack->stack_index].value.string = value;
    stack->stack_index++;
}

static inline void push_bool(kalos_stack* stack, kalos_int value) {
    stack->stack[stack->stack_index].type = KALOS_VALUE_BOOL;
    stack->stack[stack->stack_index].value.number = value;
    stack->stack_index++;
}

static inline void push_object(kalos_stack* stack, kalos_object_ref object) {
    stack->stack[stack->stack_index].type = KALOS_VALUE_OBJECT;
    stack->stack[stack->stack_index].value.object = object;
    stack->stack_index++;
}

static inline void push_none(kalos_stack* stack) {
    stack->stack[stack->stack_index].type = KALOS_VALUE_NONE;
    stack->stack_index++;
}

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
