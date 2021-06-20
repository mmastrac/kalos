#include "_kalos_defines.h"
#include "_kalos_module.h"
#include "_kalos_util.h"

void* kalos_module_get_list_item(kalos_module_parsed parsed, kalos_int offset) {
    ASSERT(offset > 0);
    if (offset == 0) {
        int x = 1;
    }
    return PTR_BYTE_OFFSET(parsed.buffer, offset);
}

kalos_module* kalos_module_find_module(kalos_module_parsed parsed, const char* name) {
    kalos_module_header* header = (kalos_module_header*)parsed.buffer;
    kalos_int module_offset = header->module_list.head;
    while (module_offset) {
        kalos_module* module = kalos_module_get_list_item(parsed, module_offset);
        if (strcmp(kalos_module_get_string(parsed, module->name_index), name) == 0) {
            return module;
        }
        module_offset = module->module_list.next;
    }
    return NULL;
}

kalos_module* kalos_module_get_module(kalos_module_parsed parsed, kalos_int module_id) {
    kalos_module_header* header = (kalos_module_header*)parsed.buffer;
    kalos_int module_offset = header->module_list.head;
    while (module_offset) {
        kalos_module* module = kalos_module_get_list_item(parsed, module_offset);
        if (module->index == module_id) {
            return module;
        }
        module_offset = module->module_list.next;
    }
    return NULL;
}

kalos_export* kalos_module_find_export(kalos_module_parsed parsed, kalos_module* module, const char* name) {
    kalos_int export_offset = module->export_list.head;
    uint16_t i = 0;
    while (export_offset) {
        kalos_export* e = kalos_module_get_list_item(parsed, export_offset);
        if (strcmp(kalos_module_get_string(parsed, e->name_index), name) == 0) {
            return e;
        }
        export_offset = e->export_list.next;
    }
    return NULL;
}

void kalos_module_walk_modules(void* context, kalos_module_parsed parsed, kalos_module_callback callback) {
    kalos_module_header* header = (kalos_module_header*)parsed.buffer;
    kalos_int module_offset = header->module_list.head;
    uint16_t i = 0;
    while (module_offset) {
        kalos_module* module = kalos_module_get_list_item(parsed, module_offset);
        if (!callback(context, parsed, i++, module)) {
            return;
        }
        module_offset = module->module_list.next;
    }
}

void kalos_module_walk_exports(void* context, kalos_module_parsed parsed, kalos_module* module, kalos_export_callback callback) {
    kalos_int export_offset = module->export_list.head;
    uint16_t i = 0;
    while (export_offset) {
        kalos_export* e = kalos_module_get_list_item(parsed, export_offset);
        if (!callback(context, parsed, i++, module, e)) {
            return;
        }
        export_offset = e->export_list.next;
    }
}

kalos_int kalos_module_lookup_property(kalos_module_parsed parsed, bool write, const char* name) {
    kalos_module_header* header = (kalos_module_header*)parsed.buffer;
    kalos_int prop_offset = header->prop_list.head;
    while (prop_offset) {
        kalos_property_address* prop_addr = kalos_module_get_list_item(parsed, prop_offset);
        if (strcmp(kalos_module_get_string(parsed, prop_addr->name_index), name) == 0) {
            kalos_int base_id = prop_addr->invoke_id * 2;
            return write ? base_id + 1 : base_id;
        }
        prop_offset = prop_addr->prop_list.next;
    }
    return 0;
}

const char* kalos_module_get_string(kalos_module_parsed parsed, kalos_int index) {
    kalos_module_header* header = (kalos_module_header*)parsed.buffer;
    return (const char *)parsed.buffer + header->string_offset + index;
}

kalos_module_header* kalos_module_get_header(kalos_module_parsed parsed) {
    return (kalos_module_header*)parsed.buffer;
}
