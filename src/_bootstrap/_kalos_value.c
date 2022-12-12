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

bool kalos_stack_setup_3(kalos_run_state* state, int arg_count, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2) {
    KALOS_CHECK__(3);
}

bool kalos_stack_setup_4(kalos_run_state* state, int arg_count, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3) {
    KALOS_CHECK__(4);
}

bool kalos_stack_setup_5(kalos_run_state* state, int arg_count, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4) {
    KALOS_CHECK__(5);
}

bool kalos_stack_setup_6(kalos_run_state* state, int arg_count, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5) {
    KALOS_CHECK__(6);
}

bool kalos_stack_setup_7(kalos_run_state* state, int arg_count, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5, kalos_value_type type6) {
    KALOS_CHECK__(7);
}

bool kalos_stack_setup_8(kalos_run_state* state, int arg_count, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5, kalos_value_type type6, kalos_value_type type7) {
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

kalos_int sized_getlength(kalos_state* state, kalos_object_ref* object) {
    struct sized_iterator_context* sized_context = (*object)->context;
    return sized_context->count;
}

kalos_value sized_getindex(kalos_state* state, kalos_object_ref* object, kalos_int index) {
    struct sized_iterator_context* sized_context = (*object)->context;
    kalos_value output = {0};
    void* ctx = PTR_BYTE_OFFSET(sized_context, sizeof(*sized_context));
    sized_context->fn(state, ctx, index, &output);
    return output;
}

kalos_object_ref kalos_allocate_sized_iterable(kalos_state* state, bool indexable, kalos_iterable_fn fn, size_t context_size, void** context, uint16_t count) {
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
    obj->getlength = sized_getlength;
    if (indexable) {
        obj->getindex = sized_getindex;
    }
    return obj;
}

void string_iterable_fn(kalos_state* state, void* context, uint16_t index, kalos_value* output) {
    const char*** ctx = context;
    output->type = KALOS_VALUE_STRING;
    output->value.string = kalos_string_allocate(state, (*ctx)[index]);
}

kalos_object_ref kalos_allocate_string_iterable(kalos_state* state, const char* values[], uint16_t count) {
    void* ctx;
    kalos_object_ref obj = kalos_allocate_sized_iterable(state, true, string_iterable_fn, sizeof(const char**), &ctx, count);
    *(const char***)ctx = values;
    return obj;
}

void int_iterable_fn(kalos_state* state, void* context, uint16_t index, kalos_value* output) {
    kalos_int** ctx = context;
    output->type = KALOS_VALUE_NUMBER;
    output->value.number = (*ctx)[index];
}

kalos_object_ref kalos_allocate_int_iterable(kalos_state* state, kalos_int values[], uint16_t count) {
    void* ctx;
    kalos_object_ref obj = kalos_allocate_sized_iterable(state, true, int_iterable_fn, sizeof(kalos_int*), &ctx, count);
    *(kalos_int**)ctx = values;
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

void list_iter_object_free(kalos_state* state, kalos_object_ref* object) {
    struct list_iter_context* context = (*object)->context;
    kalos_object_release(state, &context->object);
}

kalos_object_ref list_iterstart(kalos_state* state, kalos_object_ref* object) {
    kalos_object_ref iter = kalos_allocate_object(state, sizeof(struct list_iter_context));
    struct list_iter_context* context = iter->context;
    kalos_object_retain(state, *object);
    context->index = 0;
    context->object = *object;
    iter->iternext = list_iternext;
    iter->object_free = list_iter_object_free;
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

void list_append(kalos_state* state, kalos_object_ref* object, kalos_value* value) {
    kalos_value* array = (*object)->context;
    kalos_int length = ++array[0].value.number;
    array = (*object)->context = state->realloc(array, sizeof(*array) * (length + 1));
    array[length].type = KALOS_VALUE_NONE;
    kalos_value_move_to(state, value, &array[length]);
}

kalos_string list_tostring(kalos_state* state, kalos_object_ref* object) {
    kalos_value* array = (*object)->context;
    kalos_string out = kalos_string_allocate(state, "[");
    kalos_int n = array[0].value.number;
    for (kalos_int i = 0; i < n; i++) {
        bool comma = i > 0;
        kalos_value v = kalos_value_clone(state, &array[i + 1]);
        kalos_string item;
        if (v.type == KALOS_VALUE_STRING) {
            item = kalos_string_allocate_fmt(state, "%s\"%s\"", comma ? ", " : "", kalos_string_c(state, v.value.string));
        } else if (kalos_coerce(state, &v, KALOS_VALUE_STRING)) {
            item = kalos_string_allocate_fmt(state, "%s%s", comma ? ", " : "", kalos_string_c(state, v.value.string));
        } else {
            item = kalos_string_allocate(state, comma ? ", " : "");
        }
        out = kalos_string_take_append(state, &out, &item);
        kalos_clear(state, &v);
    }
    kalos_string end = kalos_string_allocate(state, "]");
    return kalos_string_take_append(state, &out, &end);
}

kalos_int list_getlength(kalos_state* state, kalos_object_ref* object) {
    kalos_value* array = (*object)->context;
    return array[0].value.number;
}

bool kalos_is_list(kalos_state* state, kalos_object_ref* object) {
    return (*object)->object_free == list_free;
}

kalos_object_ref kalos_list_make(kalos_state* state, kalos_int size) {
    kalos_object_ref object = kalos_allocate_object(state, 0);
    object->context = kalos_mem_alloc(state, sizeof(kalos_value) * (size + 1)); // allocate size of size
    kalos_value* array = object->context;
    array[0].type = KALOS_VALUE_NUMBER;
    array[0].value.number = size;
    object->tostring = list_tostring;
    object->getindex = list_getindex;
    object->append = list_append;
    object->getlength = list_getlength;
    object->iterstart = list_iterstart;
    object->object_free = list_free;
    return object;
}

kalos_object_ref kalos_allocate_list(kalos_state* state, kalos_int size, kalos_value* values) {
    kalos_object_ref object = kalos_list_make(state, size);
    kalos_value* array = object->context;
    memcpy(array + 1, values, sizeof(kalos_value) * size);
    for (int i = 0; i < size; i++) {
        values[i].type = KALOS_VALUE_NONE;
    }
    return object;
}

kalos_object_ref kalos_list_sublist_take(kalos_state* state, kalos_object_ref* object, kalos_int start, kalos_int end) {
    kalos_object_ref list = kalos_object_take(state, object);
    kalos_value* array = list->context;
    kalos_int length = array[0].value.number;
    kalos_clamp_range(&start, end, &length);
    // TODO
    // if (list->count == 0) {
    //     // Mutate the list in-place
    // } else {
        // Copy the list
        kalos_object_ref copy = kalos_list_make(state, length);
        kalos_value* array_copy = copy->context;
        for (kalos_int i = 0; i < length; i++) {
            array_copy[i + 1] = kalos_value_clone(state, &array[i + start + 1]);
        }
        kalos_object_release(state, &list);
        return copy;
    // }
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

#define COERCE(typ, NONE, NUMBER, OBJECT, STRING, BOOL) \
static bool coerce_##typ(kalos_state* state, kalos_value* v) {\
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
    if (v->value.object->tostring) { kalos_string tmp = v->value.object->tostring(state, &v->value.object); kalos_object_release(state, &v->value.object); v->value.string = tmp; } else { return false; },
    return true,
    v->value.string = kalos_string_allocate(state, v->value.number ? "true" : "false"))

bool kalos_coerce(kalos_state* state, kalos_value* v, kalos_value_type type) {
    bool success;
    switch (type) {
        case KALOS_VALUE_NONE:
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
    return success;
}

kalos_int kalos_hash(kalos_state* state, kalos_value* v) {
    switch (v->type) {
        case KALOS_VALUE_NONE:
            return 0;
        case KALOS_VALUE_OBJECT:
            return 0; // TODO
        case KALOS_VALUE_NUMBER:
            return v->value.number;
        case KALOS_VALUE_BOOL:
            return v->value.number;
        case KALOS_VALUE_STRING: {
            return kalos_string_hash(state, v->value.string);
        }
    }
}
