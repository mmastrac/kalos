#include "kalos_module.h"

kalos_module* kalos_module_find_module(kalos_module_parsed parsed, const char* name) {
    kalos_module_header* header = (kalos_module_header*)parsed.data;
    kalos_module* m = (kalos_module*)((uint8_t *)parsed.data + header->module_offset);
    for (uint16_t i = 0; i < header->module_count; i++) {
        if (strcmp(kalos_module_get_string(parsed, m->name_index), name) == 0) {
            return m;
        }
        m = (kalos_module*)((uint8_t *)m + sizeof(kalos_module) + m->export_count * sizeof(kalos_export));
    }
    return NULL;
}

static kalos_module_parsed* export_compare_context;
static int export_compare(const void* v1, const void* v2) {
    const char* name = v1;
    const kalos_export* e = v2;
    return strcmp(name, kalos_module_get_string(*export_compare_context, e->name_index));
}

kalos_export* kalos_module_find_export(kalos_module_parsed parsed, kalos_module* module, const char* name) {
    kalos_export* e = (kalos_export*)((uint8_t *)module + sizeof(kalos_module));
    export_compare_context = &parsed;
    // TODO: Re-entrancy/threading
    kalos_export* result = bsearch(name, e, module->export_count, sizeof(kalos_export), export_compare);
    export_compare_context = NULL;
    return result;
}

void kalos_module_walk_modules(void* context, kalos_module_parsed parsed, kalos_module_callback callback) {
    kalos_module_header* header = (kalos_module_header*)parsed.data;
    kalos_module* m = (kalos_module*)((uint8_t *)parsed.data + header->module_offset);
    for (uint16_t i = 0; i < header->module_count; i++) {
        if (!callback(context, parsed, i, m)) {
            return;
        }
        m = (kalos_module*)((uint8_t *)m + sizeof(kalos_module) + m->export_count * sizeof(kalos_export));
    }
}

void kalos_module_walk_exports(void* context, kalos_module_parsed parsed, kalos_module* module, kalos_export_callback callback) {
    kalos_export* e = (kalos_export*)((uint8_t *)module + sizeof(kalos_module));
    for (uint16_t i = 0; i < module->export_count; i++) {
        if (!callback(context, parsed, i, module, e)) {
            return;
        }
        e = (kalos_export*)((uint8_t *)e + sizeof(kalos_export));
    }
}

const char* kalos_module_get_string(kalos_module_parsed parsed, kalos_int index) {
    kalos_module_header* header = (kalos_module_header*)parsed.data;
    return (const char *)parsed.data + header->string_offset + index;
}
