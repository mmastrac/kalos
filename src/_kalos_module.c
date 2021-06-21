#include "_kalos_defines.h"
#include "_kalos_module.h"
#include "_kalos_defines.h"

void* kalos_module_get_list_item(kalos_module_parsed parsed, kalos_int offset) {
    ASSERT(offset > 0);
    if (offset == 0) {
        int x = 1;
    }
    return PTR_BYTE_OFFSET(parsed.buffer, offset);
}

kalos_module* kalos_module_find_module(kalos_module_parsed parsed, const char* name) {
    if (!parsed.buffer) {
        return NULL;
    }
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

struct kalos_module_builder_internal {
    uint8_t* kalos_module_buffer;
    int module_buffer_index, module_buffer_size;
    kalos_state* state;
};

static void* get_list_item(struct kalos_module_builder_internal* builder, kalos_int offset) {
    ASSERT(offset > 0);
    return PTR_BYTE_OFFSET(builder->kalos_module_buffer, offset);
}

static void* allocate_item(struct kalos_module_builder_internal* builder, size_t struct_size) {
    if (builder->module_buffer_index + struct_size > builder->module_buffer_size) {
        builder->module_buffer_size *= 2;
        builder->kalos_module_buffer = builder->state->realloc(builder->kalos_module_buffer, builder->module_buffer_size);
    }
    void* ptr = PTR_BYTE_OFFSET(builder->kalos_module_buffer, builder->module_buffer_index);
    memset(ptr, 0, struct_size);
    builder->module_buffer_index += struct_size;
    return ptr;
}

#define return_indexof(X) kalos_int x = ((kalos_int)PTR_BYTE_SUBTRACT(X, builder->kalos_module_buffer)); return *(kalos_module_##X*)(&x)

kalos_module_builder kalos_module_create_builder(kalos_state* state, uint8_t* buffer, size_t size) {
    struct kalos_module_builder_internal* builder = kalos_mem_alloc(state, sizeof(struct kalos_module_builder_internal));
    builder->kalos_module_buffer = buffer;
    builder->module_buffer_index = sizeof(kalos_module_header);
    builder->module_buffer_size = size;
    return (kalos_module_builder)builder;
}

void kalos_module_free_builder(kalos_state* state, kalos_module_builder builder_) {
    struct kalos_module_builder_internal* builder = builder_;
    kalos_mem_free(state, builder);
}

void kalos_module_create_idl(kalos_module_builder builder_, kalos_module_string prefix, bool dispatch_name, kalos_module_list modules, kalos_module_list prop_list) {
    struct kalos_module_builder_internal* builder = builder_;
    kalos_module_header* header = (kalos_module_header*)builder->kalos_module_buffer;
    memset(header, 0, sizeof(*header));
    header->module_list = modules;
    header->flags = dispatch_name ? KALOS_MODULE_FLAG_DISPATCH_NAME : 0;
    header->prefix_index = prefix.string_index;
    header->prop_list = prop_list;
}

kalos_module_module kalos_module_create_module(kalos_module_builder builder_, kalos_int index, kalos_module_string name, kalos_module_list exports) {
    struct kalos_module_builder_internal* builder = builder_;
    kalos_module* module = allocate_item(builder, sizeof(kalos_module));
    module->name_index = name.string_index;
    module->export_list = exports;
    module->index = index;
    return_indexof(module);
}

kalos_module_string kalos_module_create_string(kalos_module_builder builder_, const char* s) {
    struct kalos_module_builder_internal* builder = builder_;
    void* string = allocate_item(builder, strlen(s) + 1);
    strcpy(string, s);
    return_indexof(string);
}

kalos_module_list kalos_module_create_list(kalos_module_builder builder_) {
    kalos_module_list list = {0};
    return list;
}

void kalos_module_append_to_list(kalos_module_builder builder_, kalos_module_list* list, void* handle) {
    struct kalos_module_builder_internal* builder = builder_;
    kalos_int item_index = *(kalos_int*)handle;
    if (list->count == 0) {
        list->head = list->tail = item_index;
        kalos_module_item_list* item = PTR_BYTE_OFFSET(builder->kalos_module_buffer, list->tail);
        item->next = item->prev = 0;
    } else {
        kalos_module_item_list* item = PTR_BYTE_OFFSET(builder->kalos_module_buffer, list->tail);
        item->next = item_index;
        item = get_list_item(builder, item_index);
        item->prev = list->tail;
        item->next = 0;
        list->tail = item_index;
    }
    list->count++;
}

typedef int (*list_find_fn)(void* context, void* a, void* b);

static void* insert_list_item(struct kalos_module_builder_internal* builder, kalos_module_item_list_header* list, size_t struct_size, void* context, void* a, list_find_fn fn) {
    kalos_int item_index = builder->module_buffer_index;
    if (list->count > 0) {
        kalos_int item_offset = list->head;
        while (item_offset) {
            kalos_module_item_list* item = PTR_BYTE_OFFSET(builder->kalos_module_buffer, item_offset);
            int compare = fn(context, a, item);
            if (compare == 0) {
                return item; // Exact match
            } else if (compare == -1) {
                // Insert before here
                void* ptr = allocate_item(builder, struct_size);
                list->count++;
                kalos_module_item_list* new_item = ptr;
                new_item->prev = item->prev;
                new_item->next = item_offset;
                kalos_int prev_item_offset = item->prev;
                item->prev = item_index;
                if (prev_item_offset == 0) {
                    list->head = item_index;
                } else {
                    item = PTR_BYTE_OFFSET(builder->kalos_module_buffer, prev_item_offset);
                    item->next = item_index;
                }
                return ptr;
            }
            item_offset = item->next;
        }
    }
    void* ptr = allocate_item(builder, struct_size);
    kalos_module_append_to_list(builder, list, &item_index);
    return ptr;
}

kalos_module_export kalos_module_create_function_export(kalos_module_builder builder_, kalos_module_string name, kalos_module_list overrides) {
    struct kalos_module_builder_internal* builder = builder_;
    kalos_export* export = allocate_item(builder, sizeof(kalos_export));
    export->name_index = name.string_index;
    export->type = KALOS_EXPORT_TYPE_FUNCTION;
    export->entry.function_overload_list = overrides;
    return_indexof(export);
}

kalos_module_export kalos_module_create_property_export(kalos_module_builder builder_, kalos_module_string name, kalos_module_property property) {
    struct kalos_module_builder_internal* builder = builder_;
    kalos_export* export = allocate_item(builder, sizeof(kalos_export));
    export->name_index = name.string_index;
    export->type = KALOS_EXPORT_TYPE_PROPERTY;
    export->entry.property = property;
    return_indexof(export);
}

kalos_module_export kalos_module_create_const_number_export(kalos_module_builder builder_, kalos_module_string name, kalos_int value) {
    struct kalos_module_builder_internal* builder = builder_;
    kalos_export* export = allocate_item(builder, sizeof(kalos_export));
    export->name_index = name.string_index;
    export->type = KALOS_EXPORT_TYPE_CONST_NUMBER;
    export->entry.const_number = value;
    return_indexof(export);
}

kalos_module_export kalos_module_create_const_string_export(kalos_module_builder builder_, kalos_module_string name, kalos_module_string value) {
    struct kalos_module_builder_internal* builder = builder_;
    kalos_export* export = allocate_item(builder, sizeof(kalos_export));
    export->name_index = name.string_index;
    export->type = KALOS_EXPORT_TYPE_CONST_STRING;
    export->entry.const_string_index = value.string_index;
    return_indexof(export);
}

kalos_module_export kalos_module_create_handler_export(kalos_module_builder builder_, kalos_module_string name, kalos_int invoke_id, kalos_module_list args) {
    struct kalos_module_builder_internal* builder = builder_;
    kalos_export* export = allocate_item(builder, sizeof(kalos_export));
    export->name_index = name.string_index;
    export->type = KALOS_EXPORT_TYPE_HANDLER;
    export->entry.handler.arg_list = args;
    export->entry.handler.invoke_id = invoke_id;
    return_indexof(export);
}

kalos_module_export kalos_module_create_object_export(kalos_module_builder builder_, kalos_module_string name, kalos_module_list props) {
    struct kalos_module_builder_internal* builder = builder_;
    kalos_export* export = allocate_item(builder, sizeof(kalos_export));
    export->name_index = name.string_index;
    export->type = KALOS_EXPORT_TYPE_OBJECT;
    export->entry.object.prop_list = props;
    return_indexof(export);
}

kalos_module_function kalos_module_create_function(kalos_module_builder builder_, kalos_function_type type, kalos_function_type varargs, kalos_module_list args, kalos_module_binding binding) {
    struct kalos_module_builder_internal* builder = builder_;
    kalos_function* function = allocate_item(builder, sizeof(kalos_function));
    function->return_type = type;
    function->vararg_type = varargs;
    function->arg_list = args;
    function->binding = binding;
    return_indexof(function);
}

kalos_module_property kalos_module_create_property(kalos_module_builder builder_, kalos_function_type type, kalos_module_binding read, kalos_module_binding write) {
    kalos_module_property property;
    property.read = read;
    property.write = write;
    property.type = type;
    return property;
}

kalos_module_object_property kalos_module_create_object_property(kalos_module_builder builder_, kalos_module_string name, kalos_property prop) {
    struct kalos_module_builder_internal* builder = builder_;
    kalos_object_property* object_property = allocate_item(builder, sizeof(kalos_object_property));
    object_property->name_index = name.string_index;
    object_property->property = prop;
    return_indexof(object_property);
}

static int insert_prop_addr_fn(void* context, void* a, void* b) {
    struct kalos_module_builder_internal* builder = context;
    kalos_property_address* prop_addr = b;
    kalos_module_string* name = a;
    return strcmp((const char*)builder->kalos_module_buffer + name->string_index, (const char*)builder->kalos_module_buffer + prop_addr->name_index);
}

void kalos_module_add_property_address(kalos_module_builder builder_, kalos_module_list* list, kalos_module_string name) {
    kalos_property_address* property_address = insert_list_item(builder_, list, sizeof(kalos_property_address), builder_, &name, insert_prop_addr_fn);
    if (property_address->name_index == 0) {
        property_address->name_index = name.string_index;
        property_address->invoke_id = list->count;
    }
}

kalos_module_binding kalos_module_create_binding_c(kalos_module_builder builder_, kalos_int invoke_id, kalos_module_string c) {
    kalos_module_binding binding = {0};
    binding.c_index = c.string_index;
    binding.invoke_id = invoke_id;
    return binding;
}

kalos_module_binding kalos_module_create_binding_function(kalos_module_builder builder_, kalos_int invoke_id, kalos_module_string name) {
    kalos_module_binding binding = {0};
    binding.symbol_index = name.string_index;
    binding.invoke_id = invoke_id;
    return binding;
}

kalos_module_arg kalos_module_create_arg(kalos_module_builder builder_, kalos_module_string name, kalos_function_type type) {
    struct kalos_module_builder_internal* builder = builder_;
    kalos_arg* arg = allocate_item(builder, sizeof(kalos_arg));
    arg->name_index = name.string_index;
    arg->type = type;
    return_indexof(arg);
}
