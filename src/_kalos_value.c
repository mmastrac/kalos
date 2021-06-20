#include "_kalos_defines.h"
#include "_kalos_value.h"

void kalos_object_release(kalos_state* state, kalos_object_ref* ref) {
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

bool kalos_stack_setup_3(kalos_run_state* state, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2) {
    KALOS_CHECK__(3);
}

bool kalos_stack_setup_4(kalos_run_state* state, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3) {
    KALOS_CHECK__(4);
}

bool kalos_stack_setup_5(kalos_run_state* state, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4) {
    KALOS_CHECK__(5);
}

bool kalos_stack_setup_6(kalos_run_state* state, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5) {
    KALOS_CHECK__(6);
}

bool kalos_stack_setup_7(kalos_run_state* state, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5, kalos_value_type type6) {
    KALOS_CHECK__(7);
}

bool kalos_stack_setup_8(kalos_run_state* state, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5, kalos_value_type type6, kalos_value_type type7) {
    KALOS_CHECK__(8);
}

struct sized_iterator_context {
    kalos_iterable_fn fn;
    size_t context_size;
    uint16_t index;
    uint16_t count;
};

static kalos_value sized_iternext(kalos_state* state, kalos_object_ref* iter, bool* done) {
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

static kalos_object_ref sized_iterstart(kalos_state* state, kalos_object_ref* range) {
    struct sized_iterator_context* sized_context = (*range)->context;
    kalos_object_ref iter = kalos_allocate_object(state, sized_context->context_size);
    memcpy(iter->context, sized_context, sized_context->context_size);
    iter->iternext = sized_iternext;
    return iter;
}

kalos_object_ref kalos_allocate_sized_iterable(kalos_state* state, kalos_iterable_fn fn, size_t context_size, void** context, uint16_t count) {
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

struct list_iter_context {
    kalos_object_ref object;
    kalos_int index;
};

kalos_value list_iternext(kalos_state* state, kalos_object_ref* object, bool* done) {
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

kalos_object_ref list_iterstart(kalos_state* state, kalos_object_ref* object) {
    kalos_object_ref iter = kalos_allocate_object(state, sizeof(struct list_iter_context));
    struct list_iter_context* context = iter->context;
    kalos_object_retain(state, *object);
    context->index = 0;
    context->object = *object;
    iter->iternext = list_iternext;
    return iter;
}

void list_free(kalos_state* state, kalos_object_ref* object) {
    kalos_value* array = (*object)->context;
    for (int i = 0; i < array[0].value.number; i++) {
        kalos_clear(state, &array[i + 1]);
    }
    kalos_mem_free(state, array);
}

kalos_value list_getindex(kalos_state* state, kalos_object_ref* object, kalos_int index) {
    kalos_value* array = (*object)->context;
    return kalos_value_clone(state, &array[index + 1]);
}

kalos_object_ref kalos_allocate_list(kalos_state* state, kalos_int size, kalos_value* values) {
    kalos_object_ref object = kalos_allocate_object(state, 0);
    object->context = kalos_mem_alloc(state, sizeof(kalos_value) * (size + 1)); // allocate size of size
    kalos_value* array = object->context;
    array[0].type = KALOS_VALUE_NUMBER;
    array[0].value.number = size;
    memcpy(array + 1, values, sizeof(kalos_value) * size);
    for (int i = 0; i < size; i++) {
        values[i].type = KALOS_VALUE_NONE;
    }
    object->getindex = list_getindex;
    object->iterstart = list_iterstart;
    object->object_free = list_free;
    return object;
}

kalos_object_ref kalos_allocate_object(kalos_state* state, size_t context_size) {
    kalos_object_ref object = kalos_mem_alloc(state, sizeof(kalos_object) + context_size);
    memset(object, 0, sizeof(*object) + context_size);
    if (context_size) {
        object->context = (uint8_t*)object + sizeof(kalos_object);
    }
    return object;
}

kalos_object_ref kalos_allocate_prop_object(kalos_state* state, void* context, kalos_object_dispatch* dispatch) {
    kalos_object_ref object = kalos_allocate_object(state, 0);
    object->context = context;
    object->dispatch = dispatch;
    return object;
}
