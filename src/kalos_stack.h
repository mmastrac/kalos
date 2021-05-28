#pragma once

#include <stdbool.h>
#include "kalos.h"
#include "kalos_string_system.h"

typedef struct kalos_object kalos_object;
typedef struct kalos_value kalos_value;
typedef kalos_value (*kalos_propget)(kalos_state state, kalos_object* object, char* property);
typedef kalos_object* (*kalos_iterstart)(kalos_state state, kalos_object* object);
typedef kalos_value (*kalos_iternext)(kalos_state state, kalos_object* object, bool* done);
typedef void (*kalos_object_free)(kalos_state state, kalos_object* object);

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
};

typedef union kalos_value_union {
    kalos_int number;
    kalos_string string;
    kalos_object* object;
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

static void kalos_object_retain(kalos_state state, kalos_object* object) {
    object->count++;
}

static void kalos_object_release(kalos_state state, kalos_object* object) {
    if (object->count == 0) {
        if (object->object_free) {
            object->object_free(state, object);
        }
        object->count = 0x7fff;
        kalos_mem_free(state, object);
    } else {
        object->count--;
    }
}

static inline int kalos_stack_fixup_no_varargs(int arg_count, kalos_stack* stack) {
    stack->stack_index -= arg_count; 
    return 0;
}

static inline int kalos_stack_fixup_varargs(int arg_count, kalos_stack* stack) {
    int ofs = stack->stack[stack->stack_index - 1].value.number;
    stack->stack_index -= arg_count + ofs + 1;
    return ofs;
}

static inline void kalos_stack_cleanup_no_varargs(int arg_count, int ofs, kalos_state state, kalos_stack* stack) {
    for (int i = 0; i < arg_count; i++) {
        if (stack->stack[stack->stack_index + i].type == KALOS_VALUE_STRING) {
            kalos_string_release(state, stack->stack[stack->stack_index + i].value.string);
        }
    }
}

static inline void kalos_stack_cleanup_varargs(int arg_count, int ofs, kalos_state state, kalos_stack* stack) {
    for (int i = 0; i < arg_count + ofs; i++) {
        if (stack->stack[stack->stack_index + i].type == KALOS_VALUE_STRING) {
            kalos_string_release(state, stack->stack[stack->stack_index + i].value.string);
        }
    }
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

static inline void push_object(kalos_stack* stack, kalos_object* object) {
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
