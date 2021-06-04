#include <stdio.h>
#include <string.h>

#include "kalos_dump.h"
#include "kalos_parse.h"
#include "kalos_string_format.h"

static bool kalos_dump_section(void* context, kalos_script* script, kalos_export_address handle_address, uint16_t offset) {
    char** out = context;
    char* s = *out;
    if (handle_address.module_index == -1 && handle_address.export_index == -1) {
        s += sprintf(s, "<global>\n");
    } else {
        s += sprintf(s, "%04x:%04x\n", handle_address.module_index, handle_address.export_index);
    }

    for (;;) {
        s += sprintf(s, "L%04x: ", offset);
        uint8_t op = script->script_ops[offset++];
        if (op >= KALOS_OP_MAX) {
            // Don't try to continue after this.
            s += sprintf(s, "<invalid_op %02x>\n", op);
            return true;
        }
        s += sprintf(s, "%s", kalos_op_strings[op]);
        switch (op) {
            case KALOS_OP_PUSH_INTEGER: {
                uint16_t value = script->script_ops[offset++];
                value |= (uint16_t)script->script_ops[offset++] << 8;
                s += sprintf(s, " %04x", value);
                break;
            }
            case KALOS_OP_PUSH_STRING: {
                int len = strlen((char *)&script->script_ops[offset]);
                char* str = (char *)&script->script_ops[offset];
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
                        s += sprintf(s, "\\x%02x", str[i]);
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
                memcpy(&string_format, &script->script_ops[offset], sizeof(string_format));
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
        s += sprintf(s, "\n");
        if (op == KALOS_OP_END) {
            break;
        }
    }
    *out = s;
    return true;
}

void kalos_dump(kalos_script* script, char* s) {
    kalos_walk(script, &s, kalos_dump_section);
}

void kalos_walk(kalos_script* script, void* context, kalos_walk_fn walk_fn) {
    int offset = 0;
    for (;;) {
        kalos_export_address handle_address;
        memcpy(&handle_address, &script->script_ops[offset], sizeof(handle_address));
        offset += sizeof(handle_address);
        uint16_t next_section = script->script_ops[offset++];
        next_section |= script->script_ops[offset++] << 8;
        if (!walk_fn(context, script, handle_address, offset)) {
            break;
        }
        if (next_section == 0) {
            break;
        }
        offset = next_section;
    }
}

struct kalos_walk_find_section {
    kalos_export_address handle_address;
    uint16_t pc;
};

#pragma warning 303 9
static bool kalos_find_section_walk(void* context_, kalos_script* script, kalos_export_address handle_address, uint16_t pc) {
    struct kalos_walk_find_section* context = (struct kalos_walk_find_section*)context_;
    if (bcmp(&handle_address, &context->handle_address, sizeof(handle_address)) == 0) {
        context->pc = pc;
        return false;
    }
    return true;
}
#pragma warning 303 3

uint16_t kalos_find_section(kalos_script* script, kalos_export_address handle_address) {
    struct kalos_walk_find_section context = {0};
    context.handle_address = handle_address;
    kalos_walk(script, (void*)&context, kalos_find_section_walk);
    return context.pc;
}
