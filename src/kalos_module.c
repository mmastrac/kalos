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

kalos_export* kalos_module_find_export(kalos_module_parsed parsed, kalos_module* module, const char* name) {
    kalos_export* e = (kalos_export*)((uint8_t *)module + sizeof(kalos_module));
    for (uint16_t i = 0; i < module->export_count; i++) {
        if (strcmp(kalos_module_get_string(parsed, e->name_index), name) == 0) {
            return e;
        }
        e = (kalos_export*)((uint8_t *)e + sizeof(kalos_export));
    }
    return NULL;
}

void kalos_module_walk_modules(void* context, kalos_module_parsed parsed, kalos_module_callback callback) {
    kalos_module_header* header = (kalos_module_header*)parsed.data;
    kalos_module* m = (kalos_module*)((uint8_t *)parsed.data + header->module_offset);
    for (uint16_t i = 0; i < header->module_count; i++) {
        if (!callback(context, i, m)) {
            return;
        }
        m = (kalos_module*)((uint8_t *)m + sizeof(kalos_module) + m->export_count * sizeof(kalos_export));
    }
}

void kalos_module_walk_exports(void* context, kalos_module_parsed parsed, kalos_module* module, kalos_export_callback callback) {
    kalos_export* e = (kalos_export*)((uint8_t *)module + sizeof(kalos_module));
    for (uint16_t i = 0; i < module->export_count; i++) {
        if (!callback(context, i, module, e)) {
            return;
        }
        e = (kalos_export*)((uint8_t *)e + sizeof(kalos_export));
    }
}

const char* kalos_module_get_string(kalos_module_parsed parsed, kalos_int index) {
    kalos_module_header* header = (kalos_module_header*)parsed.data;
    return (const char *)parsed.data + header->string_offset + index;
}
