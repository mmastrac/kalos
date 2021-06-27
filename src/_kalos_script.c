#include "_kalos_defines.h"
#include "_kalos_script.h"
#include "_kalos_string_format.h"

#define compare_address(a, b) ((a)->module_index == (b)->module_index && (a)->export_index == (b)->export_index)

static bool kalos_dump_section(void* context, const_kalos_script script, const kalos_section_header kalos_far* header, uint16_t offset, uint16_t length) {
    char** out = context;
    char* s = *out;
    if (compare_address(&header->handler_address, &KALOS_GLOBAL_HANDLER_ADDRESS)) {
        s += sprintf(s, "<global> locals=%d\n", header->locals_size);
    } else if (compare_address(&header->handler_address, &KALOS_IDL_HANDLER_ADDRESS)) {
        s += sprintf(s, "<idl> locals=%d\n", header->locals_size);
    } else {
        s += sprintf(s, "%04x:%04x locals=%d\n", header->handler_address.module_index, header->handler_address.export_index, header->locals_size);
    }

    uint16_t start = offset;
    for (;;) {
        if (offset >= start + length) {
            break;
        }
        s += sprintf(s, "L%04x: ", offset);
        uint8_t op = script[offset++];
        if (op >= KALOS_OP_MAX) {
            // Don't try to continue after this.
            s += sprintf(s, "<invalid_op %02x>\n", op);
            return true;
        }
        s += sprintf(s, "%s", kalos_op_strings[op]);
        switch (op) {
            case KALOS_OP_PUSH_STRING: {
                int len = strlen((char *)&script[offset]);
                char* str = (char *)&script[offset];
                *s++ = ' ';
                *s++ = '"';
                for (int i = 0; i < strlen(str); i++) {
                    if (str[i] == '\n') {
                        strcpy(s, "\\n");
                        s += 2;
                    } else if (str[i] == '\"') {
                        strcpy(s, "\\\"");
                        s += 2;
                    } else if (str[i] < 32 || str[i] >= 127) {
                        s += sprintf(s, "\\x%02x", (uint8_t)str[i]);
                    } else {
                        *s++ = str[i];
                    }
                }
                *s++ = '"';
                offset += len + 1;
                break;
            }
            case KALOS_OP_FORMAT: {
                kalos_string_format string_format;
                script_memcpy(&string_format, &script[offset], sizeof(string_format));
                offset += sizeof(kalos_string_format);
                *s++ = ' ';
                *s++ = '"';
                if (string_format.fill) {
                    *s++ = string_format.fill;
                }
                if (string_format.align) {
                    *s++ = string_format.align;
                }
                if (string_format.sign) {
                    *s++ = string_format.sign;
                }
                if (string_format.alt_fmt) {
                    *s++ = string_format.alt_fmt;
                }
                if (string_format.min_width) {
                    s += sprintf(s, "%d", string_format.min_width);
                }
                if (string_format.precision) {
                    s += sprintf(s, ".%d", string_format.precision);
                }
                if (string_format.type) {
                    *s++ = string_format.type;
                }
                *s++ = '"';
                break;
            }
        }
        int inline_arg_count;
        switch (op) {
            #define KALOS_OP(op, args) case KALOS_OP_##op: inline_arg_count = args; break;
            #include "_kalos_constants.inc"
        }
        for (int i = 0; i < inline_arg_count; i++) {
            uint16_t value = script[offset++];
            value |= (uint16_t)script[offset++] << 8;
            s += sprintf(s, " %04x", value);
        }
        s += sprintf(s, "\n");
    }
    *out = s;
    return true;
}

void kalos_dump(const_kalos_script script, char* s) {
    kalos_walk(script, &s, kalos_dump_section);
}

void kalos_walk(const_kalos_script script, void* context, kalos_walk_fn walk_fn) {
    const kalos_script_header kalos_far* script_header = (const kalos_script_header kalos_far*)script;
    int offset = sizeof(*script_header);
    for (;;) {
        const kalos_section_header kalos_far* header = (const kalos_section_header kalos_far*)&script[offset];
        offset += sizeof(*header);
        uint16_t length = header->next_section == 0 ? script_header->length - offset : header->next_section - offset;
        if (!walk_fn(context, script, header, offset, length)) {
            break;
        }
        if (header->next_section == 0) {
            break;
        }
        offset = header->next_section;
    }
}

struct kalos_walk_find_section {
    kalos_export_address handler_address;
    const kalos_section_header kalos_far* header;
    uint16_t pc;
};

#pragma warning 303 9
static bool kalos_find_section_walk(void* context_, const_kalos_script script, const kalos_section_header kalos_far* header, uint16_t pc, uint16_t length) {
    struct kalos_walk_find_section* context = (struct kalos_walk_find_section*)context_;
    if (compare_address(&header->handler_address, &context->handler_address)) {
        context->pc = pc;
        context->header = header;
        return false;
    }
    return true;
}
#pragma warning 303 3

uint16_t kalos_find_section(const_kalos_script script, kalos_export_address handler_address, const kalos_section_header kalos_far** header) {
    struct kalos_walk_find_section context = {0};
    context.handler_address = handler_address;
    kalos_walk(script, (void*)&context, kalos_find_section_walk);
    if (context.pc) {
        *header = context.header;
        return context.pc;
    }
    *header = 0;
    return 0;
}

kalos_module_header* kalos_find_idl(const_kalos_script script) {
    const kalos_section_header kalos_far* header;
    kalos_int offset = kalos_find_section(script, KALOS_IDL_HANDLER_ADDRESS, &header);
    return (kalos_module_header*)(script + offset);
}
