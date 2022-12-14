#pragma once
#include "../kalos.h"

typedef enum resolve_type {
    KALOS_RESOLVE_NOT_FOUND,
    KALOS_RESOLVE_CONST_INTEGER,
    KALOS_RESOLVE_CONST_STRING,
    KALOS_RESOLVE_MODULE,
    KALOS_RESOLVE_MODULE_PROPERTY,
    KALOS_RESOLVE_OBJECT_PROPERTY,
    KALOS_RESOLVE_FUNCTION,
} resolve_type;

typedef struct resolve_dispatch {
    union {
        struct {
            kalos_int module_id_int;
            kalos_int dispatch_id_int;
        };
        struct {
            const char* module_id_str;
            const char* dispatch_id_str;
        };
    };
} resolve_dispatch;

typedef struct resolve_result {
    resolve_type type;
    union {
        kalos_int value_int;
        const char* value_str;
        void* function;
        void* module;
        struct {
            resolve_dispatch prop_read;
            resolve_dispatch prop_write;
        }
    };
} resolve_result;

resolve_result kalos_resolve(void* context, void* module, const char* name);
resolve_dispatch kalos_resolve_fn(void* context, void* function, kalos_int param_count);
