#include "defines.h"
#include "kalos_value.h"

void kalos_object_release(kalos_state state, kalos_object_ref* ref) {
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

bool kalos_stack_setup_3(kalos_stack* stack, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2) {
    KALOS_CHECK__(3);
}

bool kalos_stack_setup_4(kalos_stack* stack, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3) {
    KALOS_CHECK__(4);
}

bool kalos_stack_setup_5(kalos_stack* stack, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4) {
    KALOS_CHECK__(5);
}

bool kalos_stack_setup_6(kalos_stack* stack, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5) {
    KALOS_CHECK__(6);
}

bool kalos_stack_setup_7(kalos_stack* stack, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5, kalos_value_type type6) {
    KALOS_CHECK__(7);
}

bool kalos_stack_setup_8(kalos_stack* stack, kalos_value_type type0, kalos_value_type type1, kalos_value_type type2, kalos_value_type type3, kalos_value_type type4, kalos_value_type type5, kalos_value_type type6, kalos_value_type type7) {
    KALOS_CHECK__(8);
}
