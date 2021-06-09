#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "kalos_lex.h"
#include "kalos_module.h"
#include "kalos_parse.h"
#include "kalos_string_format.h"
#include "../defines.h"

#define KALOS_MAX_IMPORTS 16

#define KALOS_OP(x, in, out) #x ,
const char* kalos_op_strings[] = {
    #include "kalos_constants.inc"
    "<invalid>",
};

typedef struct kalos_builtin {
    const char* name;
    int param_count;
    kalos_op op;
} kalos_builtin;

// Keep sorted for binary search
#define KALOS_BUILTIN(x, y, z) { .name=#x, .param_count=y, .op=KALOS_OP_##z },
static kalos_builtin kalos_builtins[] = {
    #include "kalos_constants.inc"
    { NULL },
};

#define THROW(msg) { LOG("%d (kalos_parse.c:%d): Throw %s (last token was %s)", parse_state->lex_state.line, __LINE__, msg, kalos_token_strings[parse_state->last_token]); parse_state->failure_message = msg; goto fail; } 
#define TRY(x) {x; if (parse_state->failure_message) { LOG("%d: Caught %s", __LINE__, parse_state->failure_message); goto fail; } }
#define TRY_EXIT fail: { }
#define NO_OPERATOR_PRECEDENCE -1
#define loop for (;;)

static const char* ERROR_INVALID_TOKEN = "Invalid token";
static const char* ERROR_UNEXPECTED_TOKEN = "Unexpected token";
static const char* ERROR_UNEXPECTED_HANDLER = "Unexpected handler";
static const char* ERROR_BREAK_CONTINUE_WITHOUT_LOOP = "Break or continue but no loop";
static const char* ERROR_INTERNAL_ERROR = "Internal error";
static const char* ERROR_UNKNOWN_VARIABLE = "Unknown variable";
static const char* ERROR_UNKNOWN_HANDLE = "Unknown handle";
static const char* ERROR_UNKNOWN_PROPERTY = "Unknown property";
static const char* ERROR_INVALID_STRING_FORMAT = "Invalid string format";
static const char* ERROR_TOO_MANY_VARS = "Too many vars/consts";
static const char* ERROR_INVALID_CONST_EXPRESSION = "Invalid const expression";
static const char* ERROR_EXPECTED_MODULE = "Expected module";
static const char* ERROR_EXPECTED_FUNCTION = "Expected function";
static const char* ERROR_ILLEGAL_IN_THIS_CONTEXT = "Illegal in this context";
static const char* ERROR_DUPLICATE_IMPORT = "Duplicate import";
static const char* ERROR_UNEXPECTED_PARAMETERS = "Unexpected parameters";
static const char* ERROR_PROPERTY_NOT_WRITABLE = "Property not writable";
static const char* ERROR_PROPERTY_NOT_READABLE = "Property not readable";

#define KALOS_VAR_SLOT_COUNT 32
#define KALOS_VAR_MAX_LENGTH 16

struct var_state {
    char name[KALOS_VAR_MAX_LENGTH];
    bool is_const;
};

struct vars_state {
    struct var_state vars[KALOS_VAR_SLOT_COUNT];
    int var_index;
};

typedef struct name_resolution_result {
    enum {
        NAME_RESOLUTION_NOT_FOUND,
        NAME_RESOLUTION_BUILTIN,
        NAME_RESOLUTION_VAR,
        NAME_RESOLUTION_MODULE,
        NAME_RESOLUTION_MODULE_EXPORT,
        NAME_RESOLUTION_UNIMPORTED_MODULE,
    } type;
    union {
        struct {
            struct var_state* var;
            uint8_t var_slot;
            kalos_op load_op, store_op;
        };
        struct {
            kalos_module* module;
        };
        struct {
            uint8_t export_module_index;
            kalos_export* export;
        };
        kalos_builtin* builtin;
    };
    struct name_resolution_result* next;
    struct name_resolution_result* prev;
} name_resolution_result;

struct pending_op {
    kalos_op op;
    kalos_int data[2];
};

struct pending_ops {
    struct pending_op load, store;
    bool is_const;
    bool req_dup;
};

struct parse_state {
    kalos_lex_state lex_state;
    
    uint8_t* output_script;
    uint16_t* script_offset;
    int output_script_index;
    kalos_module_parsed all_modules;
    kalos_module* imported_modules[KALOS_MAX_IMPORTS];
    uint8_t module_index;
    kalos_module* extra_builtins;
    struct vars_state globals;
    struct vars_state locals;
    const char* failure_message;
    kalos_token last_token;
    int loop_break, loop_continue;
    int next_handler_fixup;
    char token[128];
    bool const_mode;
};

static void parse_push_format(struct parse_state* parse_state, kalos_op op, kalos_string_format* string_format);
static void parse_push_number(struct parse_state* parse_state, int number);
static void parse_push_op(struct parse_state* parse_state, kalos_op op);
static void parse_push_string(struct parse_state* parse_state, const char* s);
static void parse_push_token(struct parse_state* parse_state);
static int parse_push_goto_forward(struct parse_state* parse_state, kalos_op op);
static void parse_fixup_offset(struct parse_state* parse_state, int offset, int pc);
static void write_next_handler_section(struct parse_state* parse_state, kalos_export_address handler_address);

static bool parse_assert_token(struct parse_state* parse_state, int token);
static void parse_expression(struct parse_state* parse_state);
static void parse_expression_paren(struct parse_state* parse_state);
static void parse_expression_part(struct parse_state* parse_state);
static int parse_function_call_args(struct parse_state* parse_state);
static kalos_op parse_function_call_builtin(struct parse_state* parse_state, kalos_builtin* fn);
static struct pending_op parse_function_call_export(struct parse_state* parse_state, kalos_export* fn, uint8_t module_index);
static void parse_handler_statement(struct parse_state* parse_state);
static void parse_if_statement(struct parse_state* parse_state);
static void parse_loop_statement(struct parse_state* parse_state, bool iterator, uint8_t iterator_slot);
static void parse_statement_block(struct parse_state* parse_state);
static bool parse_statement(struct parse_state* parse_state);
static void parse_var_statement(struct parse_state* parse_state, struct vars_state* var_state);
static struct pending_ops parse_word_recursively(struct parse_state* parse_state);
static void parse_flush_pending_op(struct parse_state* parse_state, struct pending_ops* pending, bool write, bool reset);

static int lex(struct parse_state* parse_state) {
    parse_state->last_token = kalos_lex(&parse_state->lex_state, &parse_state->token[0]);
    if (parse_state->last_token == KALOS_TOKEN_ERROR) {
        LOG("c=%02x c-1=%02x %02x", *parse_state->lex_state.s, *(parse_state->lex_state.s - 1), *(parse_state->lex_state.s - 2));
        THROW(ERROR_INVALID_TOKEN);
    }
    TRY_EXIT;
    return parse_state->last_token;
}

static int lex_peek(struct parse_state* parse_state) {
    char buffer[128];
    int token = kalos_lex_peek(&parse_state->lex_state, buffer);
    if (token == KALOS_TOKEN_ERROR) {
        THROW(ERROR_INVALID_TOKEN);
    }
    TRY_EXIT;
    return token;
}

static int8_t get_operator_precedence(kalos_token token) {
    #define KALOS_TOKEN_OP(x, y, z) case KALOS_TOKEN_##y: return x;
    switch (token) {
        #include "kalos_constants.inc"
        default:
            return NO_OPERATOR_PRECEDENCE;
    }
}

static kalos_op get_operator_from_token(kalos_token token) {
    #define KALOS_TOKEN_OP(x, y, z) case KALOS_TOKEN_##y: return KALOS_OP_##z;
    switch (token) {
        #include "kalos_constants.inc"
        default:
            return KALOS_OP_MAX;
    }
}

static bool is_assignment_token(kalos_token token) {
    #define KALOS_TOKEN_OP_ASG(x, y) case KALOS_TOKEN_EQ_##x: return true;
    switch (token) {
        #include "kalos_constants.inc"
        case KALOS_TOKEN_EQ: return true;
        default:
            return false;
    }
}

static void parse_push_number(struct parse_state* parse_state, int number) {
    parse_state->output_script[parse_state->output_script_index++] = KALOS_OP_PUSH_INTEGER;
    parse_state->output_script[parse_state->output_script_index++] = number & 0xff;
    parse_state->output_script[parse_state->output_script_index++] = (number >> 8) & 0xff;
    
    LOG("NUMBER: %d", number);
}

static void parse_push_format(struct parse_state* parse_state, kalos_op op, kalos_string_format* string_format) {
    parse_state->output_script[parse_state->output_script_index++] = op;
    memcpy((void*)&parse_state->output_script[parse_state->output_script_index], (void*)string_format, sizeof(kalos_string_format));
    parse_state->output_script_index += sizeof(kalos_string_format);
}

static void parse_push_string(struct parse_state* parse_state, const char* s) {
    parse_state->output_script[parse_state->output_script_index++] = KALOS_OP_PUSH_STRING;
    strcpy((char*)&parse_state->output_script[parse_state->output_script_index], s);
    parse_state->output_script_index += strlen(s) + 1;
}

static void parse_push_token(struct parse_state* parse_state) {
    if (parse_state->last_token == KALOS_TOKEN_INTEGER) {
        TRY(parse_push_number(parse_state, atoi(parse_state->token)));
    } else if (parse_state->last_token == KALOS_TOKEN_TRUE) {
        parse_state->output_script[parse_state->output_script_index++] = KALOS_OP_PUSH_TRUE;
    } else if (parse_state->last_token == KALOS_TOKEN_FALSE) {
        parse_state->output_script[parse_state->output_script_index++] = KALOS_OP_PUSH_FALSE;
    } else if (parse_state->last_token == KALOS_TOKEN_STRING 
        || parse_state->last_token == KALOS_TOKEN_STRING_INTERPOLATION
        || parse_state->last_token == KALOS_TOKEN_WORD) {
        TRY(parse_push_string(parse_state, parse_state->token));
    } else if (parse_state->last_token == KALOS_TOKEN_STRING_FORMAT_SPEC) {
        kalos_string_format format;
        if (!kalos_parse_string_format(parse_state->token, &format)) {
            THROW(ERROR_INVALID_STRING_FORMAT);
        }
        TRY(parse_push_format(parse_state, KALOS_OP_FORMAT, &format));
    } else {
        THROW(ERROR_INTERNAL_ERROR);
    }
    LOG("TOKEN: %s %d (%s)", kalos_token_strings[parse_state->last_token], parse_state->last_token, parse_state->token);

    TRY_EXIT;
}

static void parse_push_op(struct parse_state* parse_state, kalos_op op) {
    parse_state->output_script[parse_state->output_script_index++] = op;
    LOG("OP: %s", kalos_op_strings[op]);
}

static int parse_push_goto_forward(struct parse_state* parse_state, kalos_op op) {
    TRY(parse_push_number(parse_state, 0));
    int offset = parse_state->output_script_index - 2;
    TRY(parse_push_op(parse_state, op));
    TRY_EXIT;
    return offset;
}

static void parse_fixup_offset(struct parse_state* parse_state, int offset, int pc) {
    parse_state->output_script[offset] = pc & 0xff;
    parse_state->output_script[offset + 1] = (pc >> 8) & 0xff;
    LOG("FIXUP: @%d %d", offset, pc);
}

static void write_next_handler_section(struct parse_state* parse_state, kalos_export_address handle_address) {
    if (parse_state->next_handler_fixup != 0) {
        TRY(parse_push_op(parse_state, KALOS_OP_END));
        TRY(parse_fixup_offset(parse_state, parse_state->next_handler_fixup, parse_state->output_script_index));
    }

    memcpy(&parse_state->output_script[parse_state->output_script_index], &handle_address, sizeof(handle_address));
    parse_state->output_script_index += sizeof(handle_address);

    // Next offset
    parse_state->next_handler_fixup = parse_state->output_script_index;
    parse_state->output_script[parse_state->output_script_index++] = 0;
    parse_state->output_script[parse_state->output_script_index++] = 0;

    TRY_EXIT;
}

// Resolution order: builtins, locals, globals, modules
static struct name_resolution_result resolve_word(struct parse_state* parse_state, kalos_module* context) {
    struct name_resolution_result res = { .type = NAME_RESOLUTION_NOT_FOUND, 0 };
    const char* token = parse_state->token;
    if (context) {
        kalos_export* export = kalos_module_find_export(parse_state->all_modules, context, token);
        if (export) {
            res.type = NAME_RESOLUTION_MODULE_EXPORT;
            res.export = export;
            res.export_module_index = context->index;
            return res;
        }
    } else {
        for (int i = 0; ; i++) {
            if (!kalos_builtins[i].name) {
                break;
            }
            if (strcmp(kalos_builtins[i].name, token) == 0) {
                LOG("%d: Resolved %s as builtin", parse_state->lex_state.line, token);
                res.type = NAME_RESOLUTION_BUILTIN;
                res.builtin = &kalos_builtins[i];
                return res;
            }
        }
        for (int i = 0; i < parse_state->locals.var_index; i++) {
            if (strcmp(parse_state->locals.vars[i].name, token) == 0) {
                LOG("%d: Resolved %s as local var", parse_state->lex_state.line, token);
                res.type = NAME_RESOLUTION_VAR;
                res.var = &parse_state->locals.vars[i];
                res.var_slot = i;
                res.load_op = KALOS_OP_LOAD_LOCAL;
                res.store_op = KALOS_OP_STORE_LOCAL;
                return res;
            }
        }
        for (int i = 0; i < parse_state->globals.var_index; i++) {
            if (strcmp(parse_state->globals.vars[i].name, token) == 0) {
                LOG("%d: Resolved %s as global var", parse_state->lex_state.line, token);
                res.type = NAME_RESOLUTION_VAR;
                res.var = &parse_state->globals.vars[i];
                res.var_slot = i;
                res.load_op = KALOS_OP_LOAD_GLOBAL;
                res.store_op = KALOS_OP_STORE_GLOBAL;
                return res;
            }
        }
        for (int i = 0; i < parse_state->module_index; i++) {
            kalos_module* module = parse_state->imported_modules[i];
            if (strcmp(kalos_module_get_string(parse_state->all_modules, module->name_index), token) == 0) {
                LOG("%d: Resolved %s as module", parse_state->lex_state.line, token);
                res.type = NAME_RESOLUTION_MODULE;
                res.module = module;
                return res;
            }
        }
        // Fall back to extra builtins
        if (parse_state->extra_builtins) {
            res = resolve_word(parse_state, parse_state->extra_builtins);
            if (res.type != NAME_RESOLUTION_NOT_FOUND) {
                LOG("%d: Resolved %s as extra builtin", parse_state->lex_state.line, token);
                return res;
            }
        }
        kalos_module* module = kalos_module_find_module(parse_state->all_modules, token);
        if (module) {
            LOG("%d: Resolved %s as unimported module", parse_state->lex_state.line, token);
            res.type = NAME_RESOLUTION_UNIMPORTED_MODULE;
            res.module = module;
            return res;
        }
    }

    return res;
}

static bool parse_assert_token(struct parse_state* parse_state, int token) {
    if (lex(parse_state) != token) {
        THROW(ERROR_UNEXPECTED_TOKEN);
    }
    return true;

    TRY_EXIT;
    return false;
}

static void parse_var_statement(struct parse_state* parse_state, struct vars_state* var_state) {
    if (var_state->var_index >= KALOS_VAR_SLOT_COUNT) {
        THROW(ERROR_TOO_MANY_VARS);
    }
    int slot = var_state->var_index++;
    if (parse_state->last_token == KALOS_TOKEN_CONST) {
        var_state->vars[slot].is_const = true;
    }
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
    strcpy(var_state->vars[slot].name, parse_state->token);
    kalos_token peek;
    TRY(peek = lex_peek(parse_state));
    if (peek == KALOS_TOKEN_EQ) {
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_EQ));
        if (var_state->vars[slot].is_const) {
            parse_state->const_mode = true;
        }
        TRY(parse_expression(parse_state));
        parse_state->const_mode = false;
        TRY(parse_push_number(parse_state, slot));
        TRY(parse_push_op(parse_state, var_state == &parse_state->globals ? KALOS_OP_STORE_GLOBAL : KALOS_OP_STORE_LOCAL));
    }
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
    TRY_EXIT;
}

static int parse_function_call_args(struct parse_state* parse_state) {
    kalos_token peek, param_count = 0;
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_PAREN_OPEN));
    loop {
        TRY(peek = lex_peek(parse_state));
        if (peek == KALOS_TOKEN_PAREN_CLOSE) {
            break;
        }
        TRY(parse_expression(parse_state));
        param_count++;
        TRY(peek = lex_peek(parse_state));
        if (peek == KALOS_TOKEN_COMMA) {
            TRY(lex(parse_state));
        } else if (peek != KALOS_TOKEN_PAREN_CLOSE) {
            THROW(ERROR_INVALID_TOKEN);
        }
    }
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_PAREN_CLOSE));
    TRY_EXIT;
    return param_count;
}

static kalos_op parse_function_call_builtin(struct parse_state* parse_state, kalos_builtin* fn) {
    if (parse_state->const_mode) {
        THROW(ERROR_INVALID_CONST_EXPRESSION);
    }
    int param_count = 0;
    TRY(param_count = parse_function_call_args(parse_state));
    if (param_count != fn->param_count) {
        THROW(ERROR_UNEXPECTED_PARAMETERS);
    }
    TRY_EXIT;
    return fn->op;
}

static struct pending_op parse_function_call_export(struct parse_state* parse_state, kalos_export* fn, uint8_t module_index) {
    if (parse_state->const_mode) {
        THROW(ERROR_INVALID_CONST_EXPRESSION);
    }
    int param_count = 0;
    TRY(param_count = parse_function_call_args(parse_state));
    if (fn->entry.function.vararg_type != FUNCTION_TYPE_VOID) {
        if (param_count < fn->entry.function.arg_count) {
            THROW(ERROR_UNEXPECTED_PARAMETERS);
        }
        TRY(parse_push_number(parse_state, param_count - fn->entry.function.arg_count));
    } else {
        if (param_count != fn->entry.function.arg_count) {
            THROW(ERROR_UNEXPECTED_PARAMETERS);
        }
    }
    struct pending_op op = {0};
    op.op = KALOS_OP_CALL;
    op.data[0] = module_index;
    op.data[1] = fn->entry.function.invoke_id;
    TRY_EXIT;
    return op;
}

static void parse_word_statement(struct parse_state* parse_state) {
    struct pending_ops pending;
    kalos_token peek;
    TRY(pending = parse_word_recursively(parse_state));
    TRY(peek = lex_peek(parse_state));
    if (peek == KALOS_TOKEN_EQ) {
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_EQ));
        TRY(parse_expression(parse_state));
        TRY(parse_flush_pending_op(parse_state, &pending, true, true));
        return;
    } else if (is_assignment_token(peek)) {
        TRY(parse_assert_token(parse_state, peek));
        // We'll need two copies of the data
        if (pending.req_dup) {
            TRY(parse_push_op(parse_state, KALOS_OP_DUP));
        }
        TRY(parse_flush_pending_op(parse_state, &pending, false, false));
        TRY(parse_expression(parse_state));
        kalos_op op = 0;
        #define KALOS_TOKEN_OP_ASG(x, y) case KALOS_TOKEN_EQ_##x: op = KALOS_OP_##y; break;
        switch (peek) {
            #include "kalos_constants.inc"
            default: break;
        }
        if (op) {
            TRY(parse_push_op(parse_state, op));
        }
        TRY(parse_flush_pending_op(parse_state, &pending, true, true));
        return;
    } else if (peek == KALOS_TOKEN_SEMI) {
        TRY(parse_flush_pending_op(parse_state, &pending, false, true));
        return;
    }

    THROW(ERROR_UNEXPECTED_TOKEN);

    TRY_EXIT;
}

static void parse_word_expression(struct parse_state* parse_state) {
    struct pending_ops pending;
    kalos_token peek;
    TRY(pending = parse_word_recursively(parse_state));
    TRY(parse_flush_pending_op(parse_state, &pending, false, true));
    TRY_EXIT;
}

static void parse_if_statement(struct parse_state* parse_state) {
    int fixup_offset, fixup_offset_else;
    TRY(parse_expression_paren(parse_state));
    TRY(parse_push_op(parse_state, KALOS_OP_LOGICAL_NOT));
    TRY(fixup_offset = parse_push_goto_forward(parse_state, KALOS_OP_GOTO_IF));
    TRY(parse_statement_block(parse_state));

    kalos_token peek;
    TRY(peek = lex_peek(parse_state));
    if (peek != KALOS_TOKEN_ELSE) {
        TRY(parse_fixup_offset(parse_state, fixup_offset, parse_state->output_script_index));
        return;
    }

    TRY(parse_assert_token(parse_state, KALOS_TOKEN_ELSE));
    TRY(fixup_offset_else = parse_push_goto_forward(parse_state, KALOS_OP_GOTO));
    TRY(parse_fixup_offset(parse_state, fixup_offset, parse_state->output_script_index));

    TRY(peek = lex_peek(parse_state));
    if (peek == KALOS_TOKEN_IF) {
        // Recurse for else if
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_IF));
        TRY(parse_if_statement(parse_state));
    } else {
        TRY(parse_statement_block(parse_state));
    }

    TRY(parse_fixup_offset(parse_state, fixup_offset_else, parse_state->output_script_index));

    TRY_EXIT;
}

static void parse_loop_statement(struct parse_state* parse_state, bool iterator, uint8_t iterator_slot) {
    int saved_break = parse_state->loop_break;
    int saved_continue = parse_state->loop_continue;
    int initial_fixup, break_fixup;

    // Initial entry and break
    TRY(initial_fixup = parse_push_goto_forward(parse_state, KALOS_OP_GOTO));
    parse_state->loop_break = parse_state->output_script_index;
    TRY(break_fixup = parse_push_goto_forward(parse_state, KALOS_OP_GOTO));

    TRY(parse_fixup_offset(parse_state, initial_fixup, parse_state->output_script_index));
    parse_state->loop_continue = parse_state->output_script_index;
    if (iterator) {
        TRY(parse_push_op(parse_state, KALOS_OP_ITERATOR_NEXT));
        TRY(parse_push_number(parse_state, iterator_slot));
        TRY(parse_push_op(parse_state, KALOS_OP_STORE_LOCAL));
        TRY(parse_push_number(parse_state, parse_state->loop_break));
        TRY(parse_push_op(parse_state, KALOS_OP_GOTO_IF));
    }
    TRY(parse_statement_block(parse_state));
    TRY(parse_push_number(parse_state, parse_state->loop_continue));
    TRY(parse_push_op(parse_state, KALOS_OP_GOTO));
    TRY(parse_fixup_offset(parse_state, break_fixup, parse_state->output_script_index));

    parse_state->loop_break = saved_break;
    parse_state->loop_continue = saved_break;

    TRY_EXIT;
}

static void parse_flush_pending_op(struct parse_state* parse_state, struct pending_ops* ops, bool write, bool reset) {
    struct pending_op* pending = write ? &ops->store : &ops->load;
    if (parse_state->const_mode && !ops->is_const) {
        THROW(ERROR_INVALID_CONST_EXPRESSION);
    }
    if (pending->op == KALOS_OP_LOAD_GLOBAL || pending->op == KALOS_OP_STORE_GLOBAL ||
        pending->op == KALOS_OP_LOAD_LOCAL || pending->op == KALOS_OP_STORE_LOCAL) {
        TRY(parse_push_number(parse_state, pending->data[0]));
        TRY(parse_push_op(parse_state, pending->op));
    } else if (pending->op == KALOS_OP_CALL) {
        TRY(parse_push_number(parse_state, pending->data[0]));
        TRY(parse_push_number(parse_state, pending->data[1]));
        TRY(parse_push_op(parse_state, pending->op));
    } else if (pending->op == KALOS_OP_GETPROP || pending->op == KALOS_OP_SETPROP) {
        TRY(parse_push_number(parse_state, pending->data[0]));
        TRY(parse_push_op(parse_state, pending->op));
    } else if (pending->op == KALOS_OP_PUSH_STRING) {
        TRY(parse_push_string(parse_state, kalos_module_get_string(parse_state->all_modules, pending->data[0])));
    } else if (pending->op == KALOS_OP_PUSH_INTEGER) {
        TRY(parse_push_number(parse_state, pending->data[0]));
    } else if (pending->op != 0) {
        // No associated data
        TRY(parse_push_op(parse_state, pending->op));
    } else {
        THROW(ERROR_INTERNAL_ERROR);
    }
    if (reset) {
        ops->is_const = ops->req_dup = ops->load.op = ops->store.op = 0;
    }
    TRY_EXIT;
}

static struct pending_ops parse_word_recursively(struct parse_state* parse_state) {
    // Dotted identifiers have the form |-- qualifier (0-1) --| |-- expression chain (0-n) --| |-- l-value (0-1) --|
    // Examples: module.prop.value, module.prop[foo(1)], module.fn(1,2,3).bar

    // The right-most element must be l-value compatible when used in an assignment context, but we don't
    // always know whether we're in an assignment context until we've parsed further along.
    name_resolution_result res;
    struct pending_ops pending = {0};

    TRY(res = resolve_word(parse_state, NULL));
    if (res.type == NAME_RESOLUTION_NOT_FOUND) {
        THROW(ERROR_UNKNOWN_VARIABLE);
    } else if (res.type == NAME_RESOLUTION_UNIMPORTED_MODULE) {
        THROW(ERROR_EXPECTED_MODULE);
    } else if (res.type == NAME_RESOLUTION_VAR) {
        pending.load.data[0] = pending.store.data[0] = res.var_slot;
        pending.load.op = res.load_op;
        pending.store.op = res.store_op; // consts still have a store op because they are technically vars
        pending.is_const = res.var->is_const;
    } else if (res.type == NAME_RESOLUTION_MODULE || res.type == NAME_RESOLUTION_BUILTIN || res.type == NAME_RESOLUTION_MODULE_EXPORT) {
        // These are acceptable, no pending ops
    } else {
        THROW(ERROR_INTERNAL_ERROR);
    }

    kalos_token peek;
    loop {
        TRY(peek = lex_peek(parse_state));
        if (peek == KALOS_TOKEN_PERIOD) {
            if (pending.load.op) {
                TRY(parse_flush_pending_op(parse_state, &pending, false, true));
            }
            TRY(parse_assert_token(parse_state, KALOS_TOKEN_PERIOD));
            TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
            if (res.type == NAME_RESOLUTION_MODULE) {
                // Module property
                TRY(res = resolve_word(parse_state, res.module));
                if (res.type == NAME_RESOLUTION_MODULE_EXPORT) {
                    if (res.export->type == KALOS_EXPORT_TYPE_PROPERTY) {
                        pending.load.op = pending.store.op = KALOS_OP_CALL;
                        pending.load.data[0] = pending.store.data[0] = res.export_module_index;
                        pending.load.data[1] = res.export->entry.property.read_invoke_id;
                        pending.store.data[1] = res.export->entry.property.write_invoke_id;
                    } else if (res.export->type == KALOS_EXPORT_TYPE_CONST_NUMBER) {
                        pending.is_const = true;
                        pending.load.op = KALOS_OP_PUSH_INTEGER;
                        pending.load.data[0] = res.export->entry.const_number;
                    } else if (res.export->type == KALOS_EXPORT_TYPE_CONST_STRING) {
                        pending.is_const = true;
                        pending.load.op = KALOS_OP_PUSH_STRING;
                        pending.load.data[0] = res.export->entry.const_string_index;
                    } else if (res.export->type == KALOS_EXPORT_TYPE_FUNCTION) {
                        continue;
                    } else {
                        THROW(ERROR_UNKNOWN_PROPERTY);
                    }
                } else {
                    THROW(ERROR_UNKNOWN_PROPERTY);
                }
            } else {
                // Object property
                pending.req_dup = true;
                TRY(pending.load.data[0] = kalos_module_lookup_property(parse_state->all_modules, false, parse_state->token));
                if (pending.load.data[0]) {
                    pending.load.op = KALOS_OP_GETPROP;
                }
                TRY(pending.store.data[0] = kalos_module_lookup_property(parse_state->all_modules, true, parse_state->token));
                if (pending.store.data[0]) {
                    pending.store.op = KALOS_OP_SETPROP;
                }
            }
        } else if (peek == KALOS_TOKEN_PAREN_OPEN) {
            if (res.type == NAME_RESOLUTION_BUILTIN) {
                TRY(pending.load.op = parse_function_call_builtin(parse_state, res.builtin));
            } else if (res.type == NAME_RESOLUTION_MODULE_EXPORT) {
                TRY(pending.load = parse_function_call_export(parse_state, res.export, res.export_module_index));
            }
            continue;
        } else if (peek == KALOS_TOKEN_SQBRA_OPEN) {
            // TODO: Implement indexing
            THROW(ERROR_UNEXPECTED_TOKEN);
        } else {
            break;
        }
    }
    TRY_EXIT;
    return pending;
}

static void parse_for_statement(struct parse_state* parse_state) {
    // Convert:
    //   for i in it { ... }
    // into
    //   temp = iter(it); loop { i = next(it); if (done) break; ... } drop_iterator();
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
    struct name_resolution_result res;
    TRY(res = resolve_word(parse_state, NULL));
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_IN));
    TRY(parse_expression(parse_state));
    TRY(parse_push_op(parse_state, KALOS_OP_ITERATOR));
    TRY(parse_loop_statement(parse_state, true, res.var_slot));
    TRY(parse_push_op(parse_state, KALOS_OP_DROP));
    TRY_EXIT;
}

static void parse_expression_part(struct parse_state* parse_state) {
    int token, peek;

    TRY(token = lex(parse_state));
    if (token == KALOS_TOKEN_PLUS) {
        TRY(parse_expression_part(parse_state));
        TRY(parse_push_op(parse_state, KALOS_OP_POSIVATE));
    } else if (token == KALOS_TOKEN_MINUS) {
        TRY(parse_expression_part(parse_state));
        TRY(parse_push_op(parse_state, KALOS_OP_NEGATE));
    } else if (token == KALOS_TOKEN_BANG) {
        TRY(parse_expression_part(parse_state));
        TRY(parse_push_op(parse_state, KALOS_OP_LOGICAL_NOT));
    } else if (token == KALOS_TOKEN_TILDE) {
        TRY(parse_expression_part(parse_state));
        TRY(parse_push_op(parse_state, KALOS_OP_BITWISE_NOT));
    } else if (token == KALOS_TOKEN_INTEGER || token == KALOS_TOKEN_TRUE || token == KALOS_TOKEN_FALSE) {
        TRY(parse_push_token(parse_state));
    } else if (token == KALOS_TOKEN_STRING || token == KALOS_TOKEN_STRING_INTERPOLATION) {
        bool first = true;
        loop {
            if (token == KALOS_TOKEN_STRING_INTERPOLATION) {
                bool needs_add = false;
                if (parse_state->token[0]) {
                    TRY(parse_push_token(parse_state));
                    needs_add = true;
                }
                TRY(parse_expression(parse_state));
                TRY(parse_assert_token(parse_state, KALOS_TOKEN_STRING_FORMAT_SPEC));
                if (parse_state->token[0]) {
                    TRY(parse_push_token(parse_state));
                }
                if (needs_add) {
                    TRY(parse_push_op(parse_state, KALOS_OP_ADD));
                } else {
                    TRY(parse_push_op(parse_state, KALOS_OP_TO_STRING));
                }
            } else {
                TRY(parse_push_token(parse_state));
            }
            
            if (first) {
                first = false;
            } else {
                TRY(parse_push_op(parse_state, KALOS_OP_ADD));
            }

            TRY(peek = lex_peek(parse_state));
            if (peek == KALOS_TOKEN_STRING || peek == KALOS_TOKEN_STRING_INTERPOLATION) {
                // Continue through the loop
                TRY(parse_assert_token(parse_state, peek));
                token = peek;
            } else {
                break;
            }
        }
    } else if (token == KALOS_TOKEN_WORD) {
        TRY(parse_word_expression(parse_state));
    } else if (token == KALOS_TOKEN_PAREN_OPEN) {
        TRY(parse_expression(parse_state));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_PAREN_CLOSE));
    }

    TRY_EXIT;
}

static void parse_expression(struct parse_state* parse_state) {
    kalos_token peek;
    uint8_t op_stack[16] = {0};
    uint8_t op_stack_idx = 0;

    // Start with the first element
    TRY(parse_expression_part(parse_state));

    loop {
        // Peek ahead to see if we have an operator
        TRY(peek = lex_peek(parse_state));
        int8_t precedence = get_operator_precedence(peek);
        if (precedence == NO_OPERATOR_PRECEDENCE) {
            goto exit;
        }

        TRY(lex(parse_state));
        while (op_stack_idx > 0 && precedence >= get_operator_precedence(op_stack[op_stack_idx - 1])) {
            TRY(parse_push_op(parse_state, get_operator_from_token(op_stack[--op_stack_idx])))
        }

        op_stack[op_stack_idx++] = peek;
        TRY(parse_expression_part(parse_state));
    }

    exit:
    while (op_stack_idx > 0) {
        TRY(parse_push_op(parse_state, get_operator_from_token(op_stack[--op_stack_idx])))
    }

    TRY_EXIT;
}

static void parse_expression_paren(struct parse_state* parse_state) {
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_PAREN_OPEN));
    TRY(parse_expression(parse_state));
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_PAREN_CLOSE));
    TRY_EXIT;
}

static void parse_statement_block(struct parse_state* parse_state) {
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_BRA_OPEN));
    loop {
        TRY(if (!parse_statement(parse_state)) { break; });
    }
    TRY_EXIT;
}

static bool parse_statement(struct parse_state* parse_state) {
    kalos_token peek, token;
    TRY(token = lex(parse_state));
    if (token == KALOS_TOKEN_BRA_CLOSE) {
        return false;
    } else if (token == KALOS_TOKEN_VAR || token == KALOS_TOKEN_CONST) {
        TRY(parse_var_statement(parse_state, &parse_state->locals));
    } else if (token == KALOS_TOKEN_IF) {
        TRY(parse_if_statement(parse_state));
    } else if (token == KALOS_TOKEN_LOOP) {
        TRY(parse_loop_statement(parse_state, false, 0));
    } else if (token == KALOS_TOKEN_FOR) {
        TRY(parse_for_statement(parse_state));
    } else if (token == KALOS_TOKEN_BREAK) {
        if (parse_state->loop_break == 0) {
            THROW(ERROR_BREAK_CONTINUE_WITHOUT_LOOP);
        }
        TRY(parse_push_number(parse_state, parse_state->loop_break));
        TRY(parse_push_op(parse_state, KALOS_OP_GOTO));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
    } else if (token == KALOS_TOKEN_CONTINUE) {
        if (parse_state->loop_continue == 0) {
            THROW(ERROR_BREAK_CONTINUE_WITHOUT_LOOP);
        }
        TRY(parse_push_number(parse_state, parse_state->loop_continue));
        TRY(parse_push_op(parse_state, KALOS_OP_GOTO));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
    } else if (token == KALOS_TOKEN_DEBUGGER) {
        TRY(parse_push_op(parse_state, KALOS_OP_DEBUGGER));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
    } else if (token == KALOS_TOKEN_WORD) {
        TRY(parse_word_statement(parse_state));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
    } else {
        THROW(ERROR_UNEXPECTED_TOKEN);
        return false;
    }

    TRY_EXIT;
    return true;
}

static void parse_handler_statement(struct parse_state* parse_state) {
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
    struct name_resolution_result res;
    kalos_module* context = NULL;
    kalos_export* handle = NULL;
    kalos_int module_index, handle_index;
    kalos_token peek, token;
    loop {
        TRY(res = resolve_word(parse_state, context));
        TRY(peek = lex_peek(parse_state));
        if (peek == KALOS_TOKEN_PERIOD && res.type == NAME_RESOLUTION_MODULE) {
            context = res.module;
            TRY(parse_assert_token(parse_state, KALOS_TOKEN_PERIOD));
            TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
            continue;
        }
        if (res.type == NAME_RESOLUTION_MODULE_EXPORT && res.export->type == KALOS_EXPORT_TYPE_HANDLE) {
            module_index = res.export_module_index;
            handle_index = res.export->entry.function.invoke_id;
            handle = res.export;
            break;
        }
        THROW(ERROR_UNKNOWN_HANDLE);
    }
    TRY(peek = lex_peek(parse_state));
    if (peek == KALOS_TOKEN_PAREN_OPEN) {
        struct vars_state* locals = &parse_state->locals;
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_PAREN_OPEN));
        TRY(token = lex(parse_state));
        if (token != KALOS_TOKEN_PAREN_CLOSE) {
            loop {
                if (token == KALOS_TOKEN_WORD) {
                    int slot = locals->var_index++;
                    strcpy(locals->vars[slot].name, parse_state->token);
                    TRY(token = lex(parse_state));
                    if (token == KALOS_TOKEN_COMMA) {
                        TRY(token = lex(parse_state));
                        continue;
                    } else if (token == KALOS_TOKEN_PAREN_CLOSE) {
                        break;
                    }
                }
                THROW(ERROR_UNEXPECTED_TOKEN);
            }
        }
    }
    TRY(write_next_handler_section(parse_state, kalos_make_address(module_index, handle_index)));
    TRY(parse_statement_block(parse_state));
    TRY_EXIT;
}

kalos_parse_result kalos_parse(const char kalos_far* s, kalos_module_parsed modules, kalos_script* script) {
    struct parse_state parse_state_data = {0};
    parse_state_data.output_script = script->script_ops;
    parse_state_data.all_modules = modules;
    parse_state_data.extra_builtins = kalos_module_find_module(modules, "builtin");

    kalos_lex_init(s, &parse_state_data.lex_state);
    
    struct parse_state* parse_state = &parse_state_data;

    TRY(write_next_handler_section(parse_state, KALOS_GLOBAL_HANDLE_ADDRESS));

    loop {
        int token;
        TRY(token = lex(parse_state));
        if (token == KALOS_TOKEN_EOF) {
            break;
        }

        if (token == KALOS_TOKEN_IMPORT) {
            TRY(token = lex(parse_state));
            struct name_resolution_result res;
            TRY(res = resolve_word(parse_state, NULL));
            if (res.type == NAME_RESOLUTION_UNIMPORTED_MODULE) {
                parse_state->imported_modules[parse_state->module_index++] = res.module;
            } else if (res.type == NAME_RESOLUTION_MODULE) {
                THROW(ERROR_DUPLICATE_IMPORT);
            } else {
                THROW(ERROR_EXPECTED_MODULE);
            }
            TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
        } else if (token == KALOS_TOKEN_VAR || token == KALOS_TOKEN_CONST) {
            TRY(parse_var_statement(parse_state, &parse_state->globals));
        } else if (token == KALOS_TOKEN_HANDLE) {
            memset(&parse_state->locals, 0, sizeof(parse_state->locals));
            TRY(parse_handler_statement(parse_state));
        } else {
            THROW(ERROR_UNEXPECTED_TOKEN);
        }
    }

    TRY(parse_push_op(parse_state, KALOS_OP_END));

    kalos_parse_result res = {0};
    res.success = true;
    return res;

    TRY_EXIT;
    res.success = false;
    res.error = parse_state->failure_message;
    res.line = parse_state->lex_state.line;
    return res;
}
