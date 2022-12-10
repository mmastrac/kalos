#include "_kalos_defines.h"
#include "_kalos_lex.h"
#include "_kalos_string_format.h"
#include "kalos_parse.h"

#define KALOS_MAX_IMPORTS 16

#define KALOS_OP(x, args) #x ,
const char* kalos_op_strings[] = {
    #include "_kalos_constants.inc"
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
    #include "_kalos_constants.inc"
    { NULL },
};

#define THROW(msg) { LOG("%d (kalos_parse.c:%d): Throw %s (last token was %s)", parse_state->lex_state.line, __LINE__, msg, kalos_token_strings[parse_state->last_token]); parse_state->failure_message = msg; goto fail; } 
#define TRY(x) {x; if (parse_state->failure_message) { LOG("%d: Caught %s", __LINE__, parse_state->failure_message); goto fail; } }
#define TRY_EXIT fail: { }
#define NO_OPERATOR_PRECEDENCE -1
#define loop for (;;)

static const char* ERROR_INVALID_TOKEN = "Invalid token";
static const char* ERROR_UNEXPECTED_TOKEN = "Unexpected token";
static const char* ERROR_BREAK_CONTINUE_WITHOUT_LOOP = "Break or continue but no loop";
static const char* ERROR_INTERNAL_ERROR = "Internal error";
static const char* ERROR_UNKNOWN_VARIABLE = "Unknown variable";
static const char* ERROR_UNKNOWN_HANDLE = "Unknown handler";
static const char* ERROR_UNKNOWN_PROPERTY = "Unknown property";
static const char* ERROR_UNKNOWN_FUNCTION = "Unknown function";
static const char* ERROR_MISSING_HANDLE = "Missing handler statement";
static const char* ERROR_MISSING_RETURN = "Missing return statement";
static const char* ERROR_INVALID_STRING_FORMAT = "Invalid string format";
static const char* ERROR_TOO_MANY_VARS = "Too many vars/consts";
static const char* ERROR_INVALID_CONST_EXPRESSION = "Invalid const expression";
static const char* ERROR_INVALID_IMPORT = "Invalid or missing import";
static const char* ERROR_ILLEGAL_IN_THIS_CONTEXT = "Illegal in this context";
static const char* ERROR_DUPLICATE_IMPORT = "Duplicate import";
static const char* ERROR_UNEXPECTED_PARAMETERS = "Unexpected parameters";
static const char* ERROR_UNEXPECTED_EXPORT_TYPE = "Unexpected export type";
static const char* ERROR_NO_LOADER = "No loader configured";

#define KALOS_VAR_SLOT_COUNT 64
#define KALOS_VAR_MAX_LENGTH 16

typedef enum var_type {
    VAR_EMPTY = 0,
    VAR_MUTABLE,
    VAR_CONST,
    VAR_FN,
} var_type;

struct fn_state {
    kalos_int pc;
    kalos_int param_count;
    kalos_function_type ret;
};

struct var_state {
    char name[KALOS_VAR_MAX_LENGTH];
    struct fn_state fn_state; // slot or function index
    var_type type;
    bool export;
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
} name_resolution_result;

struct pending_op {
    kalos_op op;
    kalos_int data[3];
    const char* sdata[2];
};

struct pending_ops {
    struct pending_op load, store;
    bool is_const;
    bool req_dup;
};

struct parse_state {
    kalos_state* state;
    kalos_lex_state lex_state;
    kalos_parse_options options;
    bool dispatch_name;
    uint8_t kalos_far* output_script;
    uint16_t* script_offset;
    uint16_t function_invoke_id;
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
    kalos_section_header* last_section_header;
    char token[256];
    bool const_mode;
    kalos_function_type ret;
    kalos_token last_statement_type;
};

static void parse_push_format(struct parse_state* parse_state, kalos_op op, kalos_string_format* string_format);
static void parse_push_op(struct parse_state* parse_state, kalos_op op);
static void parse_push_op_1(struct parse_state* parse_state, kalos_op op, kalos_int number);
static void parse_push_string(struct parse_state* parse_state, const char* s);
static void parse_push_token(struct parse_state* parse_state);
static int parse_push_goto_forward(struct parse_state* parse_state, kalos_op op);
static void parse_fixup_offset(struct parse_state* parse_state, int offset, kalos_int pc);
static void write_next_handler_section(struct parse_state* parse_state, kalos_export_address handler_address);

static void parse_assert_token(struct parse_state* parse_state, int token);
static void parse_expression(struct parse_state* parse_state);
static void parse_expression_part(struct parse_state* parse_state);
static int parse_list_of_args(struct parse_state* parse_state, kalos_token open, kalos_token close);
static struct pending_op parse_function_call_local(struct parse_state* parse_state, struct var_state* fn);
static kalos_op parse_function_call_builtin(struct parse_state* parse_state, kalos_builtin* fn);
static struct pending_op parse_function_call_export(struct parse_state* parse_state, kalos_export* fn, uint8_t module_index);
static kalos_function_type parse_map_type(struct parse_state* parse_state, kalos_token token);
static void parse_function_statement(struct parse_state* parse_state, bool export);
static void parse_if_statement(struct parse_state* parse_state);
static void parse_loop_statement(struct parse_state* parse_state, bool iterator, uint8_t iterator_slot);
static void parse_statement_block(struct parse_state* parse_state);
static bool parse_statement(struct parse_state* parse_state);
static int parse_var_allocate(struct parse_state* parse_state, struct vars_state* var_state);
static void parse_var_statement(struct parse_state* parse_state, struct vars_state* var_state, bool export);
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
    char buffer[256];
    kalos_token token = kalos_lex_peek(&parse_state->lex_state, buffer);
    if (token == KALOS_TOKEN_ERROR) {
        THROW(ERROR_INVALID_TOKEN);
    }
    TRY_EXIT;
    return token;
}

static int8_t get_operator_precedence(kalos_token token) {
    #define KALOS_TOKEN_OP(x, y, z) case KALOS_TOKEN_##y: return x;
    switch (token) {
        #include "_kalos_constants.inc"
        default:
            return NO_OPERATOR_PRECEDENCE;
    }
}

static kalos_op get_operator_from_token(kalos_token token) {
    #define KALOS_TOKEN_OP(x, y, z) case KALOS_TOKEN_##y: return KALOS_OP_##z;
    switch (token) {
        #include "_kalos_constants.inc"
        default:
            return KALOS_OP_MAX;
    }
}

static bool is_assignment_token(kalos_token token) {
    #define KALOS_TOKEN_OP_ASG(x, y) case KALOS_TOKEN_EQ_##x: return true;
    switch (token) {
        #include "_kalos_constants.inc"
        case KALOS_TOKEN_EQ: return true;
        default:
            return false;
    }
}

/********************************************************************
 * Script modification
 * 
 * All functions in this section must correctly account for data written
 * to the script so that the parser can compile in "sizing mode".
 ********************************************************************/

static bool parse_allocate_space(struct parse_state* parse_state, kalos_int size) {
    if (parse_state->output_script) {
        return true;
    }
    parse_state->output_script_index += size;
    return false;
}

static void parse_fixup_offset(struct parse_state* parse_state, int offset, kalos_int pc) {
    if (parse_allocate_space(parse_state, 0)) {
        parse_state->output_script[offset] = pc & 0xff;
        parse_state->output_script[offset + 1] = (pc >> 8) & 0xff;
        LOG("FIXUP: @%d %d", offset, pc);
    }
}

static void write_next_handler_section(struct parse_state* parse_state, kalos_export_address handler_address) {
    if (parse_allocate_space(parse_state, sizeof(*parse_state->last_section_header))) {
        if (parse_state->last_section_header) {
            parse_state->last_section_header->next_section = parse_state->output_script_index;
        }

        parse_state->last_section_header = (kalos_section_header*)&parse_state->output_script[parse_state->output_script_index];
        memset(parse_state->last_section_header, 0, sizeof(*parse_state->last_section_header));
        parse_state->last_section_header->handler_address = handler_address;
        parse_state->output_script_index += sizeof(*parse_state->last_section_header);
    }
}

static void parse_push_format(struct parse_state* parse_state, kalos_op op, kalos_string_format* string_format) {
    if (parse_allocate_space(parse_state, sizeof(kalos_string_format) + 1)) {
        parse_state->output_script[parse_state->output_script_index++] = op;
        memcpy((void*)&parse_state->output_script[parse_state->output_script_index], (void*)string_format, sizeof(kalos_string_format));
        parse_state->output_script_index += sizeof(kalos_string_format);
    }
}

static void parse_push_string(struct parse_state* parse_state, const char* s) {
    kalos_int len = strlen(s);
    if (parse_allocate_space(parse_state, len + 2)) {
        parse_state->output_script[parse_state->output_script_index++] = KALOS_OP_PUSH_STRING;
        strcpy((char*)&parse_state->output_script[parse_state->output_script_index], s);
        parse_state->output_script_index += len + 1;
    }
}

static void parse_push_op(struct parse_state* parse_state, kalos_op op) {
    if (parse_allocate_space(parse_state, 1)) {
        parse_state->output_script[parse_state->output_script_index++] = op;
        LOG("OP: %s", kalos_op_strings[op]);
    }
}

static void parse_push_op_1(struct parse_state* parse_state, kalos_op op, kalos_int data) {
    if (parse_allocate_space(parse_state, 3)) {
        parse_state->output_script[parse_state->output_script_index++] = op;
        LOG("OP: %s %d", kalos_op_strings[op], data);
        parse_state->output_script[parse_state->output_script_index++] = data & 0xff;
        parse_state->output_script[parse_state->output_script_index++] = (data >> 8) & 0xff;
    }
}

static void parse_push_op_2(struct parse_state* parse_state, kalos_op op, kalos_int data1, kalos_int data2) {
    if (parse_allocate_space(parse_state, 5)) {
        parse_state->output_script[parse_state->output_script_index++] = op;
        LOG("OP: %s %d %d", kalos_op_strings[op], data1, data2);
        parse_state->output_script[parse_state->output_script_index++] = data1 & 0xff;
        parse_state->output_script[parse_state->output_script_index++] = (data1 >> 8) & 0xff;
        parse_state->output_script[parse_state->output_script_index++] = data2 & 0xff;
        parse_state->output_script[parse_state->output_script_index++] = (data2 >> 8) & 0xff;
    }
}

/********************************************************************
 * End script modification
 ********************************************************************/

static void parse_push_token(struct parse_state* parse_state) {
    if (parse_state->last_token == KALOS_TOKEN_INTEGER) {
        // This is legal because it'll be a NUL byte for a base-10 zero
        if (parse_state->token[1] == 'x' || parse_state->token[1] == 'X') {
            TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, strtol(parse_state->token + 2, NULL, 16)));
        } else {
            TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, atoi(parse_state->token)));
        }
    } else if (parse_state->last_token == KALOS_TOKEN_TRUE) {
        TRY(parse_push_op(parse_state, KALOS_OP_PUSH_TRUE));
    } else if (parse_state->last_token == KALOS_TOKEN_FALSE) {
        TRY(parse_push_op(parse_state, KALOS_OP_PUSH_FALSE));
    } else if (parse_state->last_token == KALOS_TOKEN_STRING_LITERAL 
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

static void parse_make_list(struct parse_state* parse_state, kalos_int size) {
    TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, size));
    TRY(parse_push_op(parse_state, KALOS_OP_MAKE_LIST));
    TRY_EXIT;
}

static int parse_push_goto_forward(struct parse_state* parse_state, kalos_op op) {
    TRY(parse_push_op_1(parse_state, op, 0));
    TRY_EXIT;
    return parse_state->output_script_index - 2;
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

static kalos_token wordify(struct parse_state* parse_state) {
    switch (parse_state->last_token) {
    #define KALOS_TOKEN_WORD(x) case KALOS_TOKEN_##x:
    #include "_kalos_constants.inc"
    {
        kalos_int len = parse_state->lex_state.s - parse_state->lex_state.token_start;
        script_memcpy(parse_state->token, (const uint8_t kalos_far*)parse_state->lex_state.token_start, len);
        parse_state->token[len] = 0;
        parse_state->last_token = KALOS_TOKEN_WORD;
        return KALOS_TOKEN_WORD;
    }
    default:
        return parse_state->last_token;
    }
}

static void parse_assert_token(struct parse_state* parse_state, int token) {
    if (lex(parse_state) != token) {
        if (token == KALOS_TOKEN_WORD) {
            if (wordify(parse_state) == KALOS_TOKEN_WORD) {
                return;
            }
        }
        THROW(ERROR_UNEXPECTED_TOKEN);
    }
    TRY_EXIT;
}

static int parse_var_allocate(struct parse_state* parse_state, struct vars_state* var_state) {
    if (var_state->var_index >= KALOS_VAR_SLOT_COUNT) {
        THROW(ERROR_TOO_MANY_VARS);
    }
    int slot = var_state->var_index++;
    if (parse_state->last_token == KALOS_TOKEN_CONST) {
        var_state->vars[slot].type = VAR_CONST;
    } else if (parse_state->last_token == KALOS_TOKEN_VAR) {
        var_state->vars[slot].type = VAR_MUTABLE;
    } else if (parse_state->last_token == KALOS_TOKEN_FN) {
        var_state->vars[slot].type = VAR_FN;
    } else {
        THROW(ERROR_UNEXPECTED_TOKEN);
    }
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
    strcpy(var_state->vars[slot].name, parse_state->token);
    TRY_EXIT;
    return slot;
}

static void parse_var_statement(struct parse_state* parse_state, struct vars_state* var_state, bool export) {
    int slot;
    TRY(slot = parse_var_allocate(parse_state, var_state));
    kalos_token peek;
    TRY(peek = lex_peek(parse_state));
    if (peek == KALOS_TOKEN_EQ) {
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_EQ));
        if (var_state->vars[slot].type == VAR_CONST) {
            parse_state->const_mode = true;
        }
        TRY(parse_expression(parse_state));
        parse_state->const_mode = false;
        TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, slot));
        TRY(parse_push_op(parse_state, var_state == &parse_state->globals ? KALOS_OP_STORE_GLOBAL : KALOS_OP_STORE_LOCAL));
    }
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
    TRY_EXIT;
}

static int parse_list_of_args(struct parse_state* parse_state, kalos_token open, kalos_token close) {
    kalos_token peek, param_count = 0;
    if (open) {
        TRY(parse_assert_token(parse_state, open));
    }
    loop {
        TRY(peek = lex_peek(parse_state));
        if (peek == close) {
            break;
        }
        TRY(parse_expression(parse_state));
        param_count++;
        TRY(peek = lex_peek(parse_state));
        if (peek == KALOS_TOKEN_COMMA) {
            TRY(lex(parse_state));
        } else if (peek != close) {
            THROW(ERROR_INVALID_TOKEN);
        }
    }
    TRY(parse_assert_token(parse_state, close));
    TRY_EXIT;
    return param_count;
}

static struct pending_op parse_function_call_local(struct parse_state* parse_state, struct var_state* fn) {
    if (parse_state->const_mode) {
        THROW(ERROR_INVALID_CONST_EXPRESSION);
    }
    int param_count = 0;
    TRY(param_count = parse_list_of_args(parse_state, KALOS_TOKEN_PAREN_OPEN, KALOS_TOKEN_PAREN_CLOSE));
    if (param_count != fn->fn_state.param_count) {
        THROW(ERROR_UNEXPECTED_PARAMETERS);
    }
    TRY_EXIT;
    struct pending_op op = {0};
    op.op = KALOS_OP_GOSUB;
    op.data[0] = fn->fn_state.param_count;
    op.data[1] = fn->fn_state.pc;
    return op;
}

static kalos_op parse_function_call_builtin(struct parse_state* parse_state, kalos_builtin* fn) {
    if (parse_state->const_mode) {
        THROW(ERROR_INVALID_CONST_EXPRESSION);
    }
    int param_count = 0;
    TRY(param_count = parse_list_of_args(parse_state, KALOS_TOKEN_PAREN_OPEN, KALOS_TOKEN_PAREN_CLOSE));
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
    TRY(param_count = parse_list_of_args(parse_state, KALOS_TOKEN_PAREN_OPEN, KALOS_TOKEN_PAREN_CLOSE));
    kalos_function* overload = kalos_module_get_list_item(parse_state->all_modules, fn->entry.function_overload_list.head);
    while (overload) {
        if (overload->vararg_type != FUNCTION_TYPE_VOID) {
            if (param_count >= overload->arg_list.count) {
                TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, param_count - overload->arg_list.count));
                goto found;
            }
        } else {
            if (param_count == overload->arg_list.count) {
                goto found;
            }
        }
        overload = overload->fn_overload_list.next ? kalos_module_get_list_item(parse_state->all_modules, overload->fn_overload_list.next) : NULL; 
    }
    THROW(ERROR_UNEXPECTED_PARAMETERS);
    found: ;
    struct pending_op op = {0};
    op.op = KALOS_OP_CALL;
    op.data[0] = module_index;
    op.data[1] = overload->binding.invoke_id;
    op.data[2] = overload->arg_list.count;
    if (parse_state->dispatch_name) {
        op.sdata[0] = kalos_module_get_string(parse_state->all_modules, kalos_module_get_module(parse_state->all_modules, module_index)->name_index);
        op.sdata[1] = kalos_module_get_string(parse_state->all_modules, fn->name_index);
    }
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
            #include "_kalos_constants.inc"
            default: break;
        }
        if (op) {
            TRY(parse_push_op(parse_state, op));
        }
        TRY(parse_flush_pending_op(parse_state, &pending, true, true));
        return;
    } else if (peek == KALOS_TOKEN_SEMI && pending.load.op == KALOS_OP_CALL) {
        pending.load.op = KALOS_OP_CALL_NORET;
        TRY(parse_flush_pending_op(parse_state, &pending, false, true));
        return;
    } else if (peek == KALOS_TOKEN_SEMI && pending.load.op == KALOS_OP_GOSUB) {
        pending.load.op = KALOS_OP_GOSUB_NORET;
        TRY(parse_flush_pending_op(parse_state, &pending, false, true));
        return;
    }

    THROW(ERROR_UNEXPECTED_TOKEN);

    TRY_EXIT;
}

static void parse_word_expression(struct parse_state* parse_state) {
    struct pending_ops pending;
    TRY(pending = parse_word_recursively(parse_state));
    TRY(parse_flush_pending_op(parse_state, &pending, false, true));
    TRY_EXIT;
}

static void parse_if_statement(struct parse_state* parse_state) {
    int fixup_offset, fixup_offset_else;
    TRY(parse_expression(parse_state));
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
        TRY(parse_push_op_2(parse_state, KALOS_OP_ITERATOR_NEXT, iterator_slot, parse_state->loop_break));
    }
    TRY(parse_statement_block(parse_state));
    TRY(parse_push_op_1(parse_state, KALOS_OP_GOTO, parse_state->loop_continue));
    TRY(parse_fixup_offset(parse_state, break_fixup, parse_state->output_script_index));

    parse_state->loop_break = saved_break;
    parse_state->loop_continue = saved_continue;

    TRY_EXIT;
}

static void parse_flush_pending_op(struct parse_state* parse_state, struct pending_ops* ops, bool write, bool reset) {
    struct pending_op* pending = write ? &ops->store : &ops->load;
    if (parse_state->const_mode && !ops->is_const) {
        THROW(ERROR_INVALID_CONST_EXPRESSION);
    }
    if (pending->op == KALOS_OP_LOAD_GLOBAL || pending->op == KALOS_OP_STORE_GLOBAL ||
        pending->op == KALOS_OP_LOAD_LOCAL || pending->op == KALOS_OP_STORE_LOCAL) {
        TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, pending->data[0]));
        TRY(parse_push_op(parse_state, pending->op));
    } else if (pending->op == KALOS_OP_CALL || pending->op == KALOS_OP_CALL_NORET) {
        if (parse_state->dispatch_name) {
            TRY(parse_push_string(parse_state, pending->sdata[0]));
            TRY(parse_push_string(parse_state, pending->sdata[1]));
            TRY(parse_push_op_1(parse_state, pending->op == KALOS_OP_CALL ? KALOS_OP_CALL_BYNAME : KALOS_OP_CALL_BYNAME_NORET, pending->data[2]));
        } else {
            TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, pending->data[0]));
            TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, pending->data[1]));
            TRY(parse_push_op_1(parse_state, pending->op, pending->data[2]));
        }
    } else if (pending->op == KALOS_OP_GOSUB || pending->op == KALOS_OP_GOSUB_NORET) {
        TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, pending->data[0]));
        TRY(parse_push_op_1(parse_state, pending->op, pending->data[1]));
    } else if (pending->op == KALOS_OP_OBJCALL) {
        if (parse_state->dispatch_name) {
            TRY(parse_push_string(parse_state, pending->sdata[0]));
            TRY(parse_push_op_1(parse_state, KALOS_OP_OBJCALL_BYNAME, pending->data[1]));
        } else {
            TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, pending->data[0]));
            TRY(parse_push_op_1(parse_state, pending->op, pending->data[1]));
        }
    } else if (pending->op == KALOS_OP_PUSH_STRING) {
        TRY(parse_push_string(parse_state, kalos_module_get_string(parse_state->all_modules, pending->data[0])));
    } else if (pending->op == KALOS_OP_PUSH_INTEGER) {
        TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, pending->data[0]));
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
        THROW(ERROR_INVALID_IMPORT);
    } else if (res.type == NAME_RESOLUTION_VAR) {
        pending.load.data[0] = pending.store.data[0] = res.var_slot;
        pending.load.op = res.load_op;
        pending.store.op = res.store_op; // consts still have a store op because they are technically vars
        pending.is_const = res.var->type == VAR_CONST;
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
                        if (parse_state->dispatch_name) {
                            pending.load.sdata[0] = pending.store.sdata[0] = kalos_module_get_string(parse_state->all_modules, kalos_module_get_module(parse_state->all_modules, res.export_module_index)->name_index);
                            pending.load.sdata[1] = kalos_module_get_string(parse_state->all_modules, res.export->name_index);
                            pending.store.sdata[1] = kalos_module_get_string(parse_state->all_modules, res.export->name_index);
                        }
                        pending.load.data[1] = res.export->entry.property.read.invoke_id;
                        pending.load.data[2] = 0;
                        pending.store.data[1] = res.export->entry.property.write.invoke_id;
                        pending.store.data[2] = 1;
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
                    pending.load.sdata[0] = parse_state->token;
                    pending.store.data[1] = 0;
                    pending.load.op = KALOS_OP_OBJCALL;
                }
                TRY(pending.store.data[0] = kalos_module_lookup_property(parse_state->all_modules, true, parse_state->token));
                if (pending.store.data[0]) {
                    pending.store.data[1] = 1;
                    pending.store.sdata[0] = parse_state->token;
                    pending.store.op = KALOS_OP_OBJCALL;
                }
                if (!pending.load.op && !pending.store.op) {
                    THROW(ERROR_UNKNOWN_PROPERTY);
                }
            }
        } else if (peek == KALOS_TOKEN_PAREN_OPEN) {
            if (res.type == NAME_RESOLUTION_BUILTIN) {
                TRY(pending.load.op = parse_function_call_builtin(parse_state, res.builtin));
            } else if (res.type == NAME_RESOLUTION_MODULE_EXPORT) {
                TRY(pending.load = parse_function_call_export(parse_state, res.export, res.export_module_index));
            } else if (res.type == NAME_RESOLUTION_VAR && res.var->type == VAR_FN) {
                TRY(pending.load = parse_function_call_local(parse_state, res.var));
            } else {
                THROW(ERROR_UNKNOWN_FUNCTION);
            }
            continue;
        } else if (peek == KALOS_TOKEN_SQBRA_OPEN) {
            if (pending.load.op) {
                TRY(parse_flush_pending_op(parse_state, &pending, false, true));
            }
            TRY(parse_assert_token(parse_state, peek));
            TRY(parse_expression(parse_state));
            TRY(parse_assert_token(parse_state, KALOS_TOKEN_SQBRA_CLOSE));
            pending.load.op = KALOS_OP_GETINDEX;
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
    if (res.type == NAME_RESOLUTION_NOT_FOUND) {
        THROW(ERROR_UNKNOWN_VARIABLE);
    }
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_IN));
    TRY(parse_expression(parse_state));
    TRY(parse_push_op(parse_state, KALOS_OP_ITERATOR));
    TRY(parse_loop_statement(parse_state, true, res.var_slot));
    TRY(parse_push_op(parse_state, KALOS_OP_DROP));
    TRY_EXIT;
}

static void parse_string_expression(struct parse_state* parse_state, kalos_token token) {
    if (token != KALOS_TOKEN_STRING_LITERAL && token != KALOS_TOKEN_STRING_INTERPOLATION) {
        THROW(ERROR_UNEXPECTED_TOKEN);
    }
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

        kalos_token peek;
        TRY(peek = lex_peek(parse_state));
        if (peek == KALOS_TOKEN_STRING_LITERAL || peek == KALOS_TOKEN_STRING_INTERPOLATION) {
            // Continue through the loop
            TRY(parse_assert_token(parse_state, peek));
            token = peek;
        } else {
            break;
        }
    }
    TRY_EXIT;
}

static void parse_expression_part(struct parse_state* parse_state) {
    kalos_token token;

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
    } else if (token == KALOS_TOKEN_STRING_LITERAL || token == KALOS_TOKEN_STRING_INTERPOLATION) {
        TRY(parse_string_expression(parse_state, token));
    } else if (wordify(parse_state) == KALOS_TOKEN_WORD) {
        TRY(parse_word_expression(parse_state));
    } else if (token == KALOS_TOKEN_PAREN_OPEN) {
        TRY(parse_expression(parse_state));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_PAREN_CLOSE));
    } else if (token == KALOS_TOKEN_SQBRA_OPEN) {
        int count;
        TRY(count = parse_list_of_args(parse_state, 0, KALOS_TOKEN_SQBRA_CLOSE));
        TRY(parse_make_list(parse_state, count));
    } else if (token == KALOS_TOKEN_AT_AT) {
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_PAREN_OPEN));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_PERIOD));
        TRY(token = lex(parse_state));
        if (wordify(parse_state) != KALOS_TOKEN_WORD) {
            THROW(ERROR_UNKNOWN_PROPERTY);
        }
        kalos_int property = kalos_module_lookup_property(parse_state->all_modules, false, parse_state->token);
        if (parse_state->dispatch_name) {
            TRY(parse_push_string(parse_state, parse_state->token));
        } else {
            TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, property));
        }
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_PAREN_CLOSE));
    }

    TRY_EXIT;
}

static void parse_expression(struct parse_state* parse_state) {
    kalos_token peek;
    kalos_token op_stack[16] = {0};
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

        TRY(parse_assert_token(parse_state, peek));
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

static void parse_statement_block(struct parse_state* parse_state) {
    parse_state->last_statement_type = KALOS_TOKEN_ERROR;
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
    }
    parse_state->last_statement_type = token;
    if (token == KALOS_TOKEN_VAR || token == KALOS_TOKEN_CONST) {
        TRY(parse_var_statement(parse_state, &parse_state->locals, false));
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
        TRY(parse_push_op_1(parse_state, KALOS_OP_GOTO, parse_state->loop_break));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
    } else if (token == KALOS_TOKEN_CONTINUE) {
        if (parse_state->loop_continue == 0) {
            THROW(ERROR_BREAK_CONTINUE_WITHOUT_LOOP);
        }
        TRY(parse_push_op_1(parse_state, KALOS_OP_GOTO, parse_state->loop_continue));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
    } else if (token == KALOS_TOKEN_DEBUGGER) {
        TRY(parse_push_op(parse_state, KALOS_OP_DEBUGGER));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
    } else if (token == KALOS_TOKEN_RETURN) {
        TRY(peek = lex_peek(parse_state));
        if (peek != KALOS_TOKEN_SEMI) {
            if (parse_state->ret == FUNCTION_TYPE_VOID) {
                THROW(ERROR_ILLEGAL_IN_THIS_CONTEXT)
            }
            TRY(parse_expression(parse_state));
        } else {
            if (parse_state->ret != FUNCTION_TYPE_VOID) {
                THROW(ERROR_ILLEGAL_IN_THIS_CONTEXT)
            }
        }
        TRY(parse_push_op(parse_state, KALOS_OP_END));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
    } else if (wordify(parse_state) == KALOS_TOKEN_WORD) {
        TRY(parse_word_statement(parse_state));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
    } else {
        THROW(ERROR_UNEXPECTED_TOKEN);
        return false;
    }

    TRY_EXIT;
    return true;
}

static void parse_function_statement(struct parse_state* parse_state, bool export) {
    kalos_token peek, token = parse_state->last_token;
    kalos_int module_index = 0, handler_index = 0;
    struct var_state* var = NULL;
    if (token == KALOS_TOKEN_ON) {
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
        struct name_resolution_result res;
        kalos_module* context = NULL;
        kalos_export* handler = NULL;
        loop {
            TRY(res = resolve_word(parse_state, context));
            TRY(peek = lex_peek(parse_state));
            if (peek == KALOS_TOKEN_PERIOD && res.type == NAME_RESOLUTION_MODULE) {
                context = res.module;
                TRY(parse_assert_token(parse_state, KALOS_TOKEN_PERIOD));
                TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
                continue;
            }
            if (res.type == NAME_RESOLUTION_MODULE_EXPORT && res.export->type == KALOS_EXPORT_TYPE_HANDLER) {
                module_index = res.export_module_index;
                handler_index = res.export->entry.handler.invoke_id;
                handler = res.export;
                break;
            }
            THROW(ERROR_UNKNOWN_HANDLE);
        }
    } else {
        int slot;
        TRY(slot = parse_var_allocate(parse_state, &parse_state->globals));
        var = &(parse_state->globals.vars[slot]);
    }
    int param_count = 0;
    TRY(peek = lex_peek(parse_state));
    if (peek == KALOS_TOKEN_PAREN_OPEN) {
        struct vars_state* locals = &parse_state->locals;
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_PAREN_OPEN));
        TRY(token = lex(parse_state));
        if (token != KALOS_TOKEN_PAREN_CLOSE) {
            loop {
                if (wordify(parse_state) == KALOS_TOKEN_WORD) {
                    int slot = locals->var_index++;
                    param_count++;
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
    if (var) {
        // Function return type
        var->fn_state.pc = parse_state->output_script_index;
        var->fn_state.param_count = param_count;
        TRY(peek = lex_peek(parse_state));
        if (peek == KALOS_TOKEN_COLON) {
            TRY(parse_assert_token(parse_state, KALOS_TOKEN_COLON));
            TRY(token = lex(parse_state));
            TRY(parse_state->ret = var->fn_state.ret = parse_map_type(parse_state, token));
        }
    } else {
        // Handler return type
        TRY(peek = lex_peek(parse_state));
        if (peek == KALOS_TOKEN_COLON) {
            TRY(parse_assert_token(parse_state, KALOS_TOKEN_COLON));
            TRY(token = lex(parse_state));
            TRY(parse_state->ret = parse_map_type(parse_state, token));
        }
    }
    TRY(write_next_handler_section(parse_state, kalos_make_address(module_index, handler_index)));
    TRY(parse_statement_block(parse_state));
    if (parse_state->ret != FUNCTION_TYPE_VOID) {
        if (parse_state->last_statement_type != KALOS_TOKEN_RETURN) {
            THROW(ERROR_MISSING_RETURN);
        }
        parse_state->ret = FUNCTION_TYPE_VOID;
    }
    TRY(parse_push_op(parse_state, KALOS_OP_END));
    TRY_EXIT;
}

kalos_function_type parse_map_type(struct parse_state* parse_state, kalos_token token) {
    switch (token) {
        case KALOS_TOKEN_VOID: return FUNCTION_TYPE_VOID;
        case KALOS_TOKEN_STRING: return FUNCTION_TYPE_STRING;
        case KALOS_TOKEN_OBJECT: return FUNCTION_TYPE_OBJECT;
        case KALOS_TOKEN_BOOL: return FUNCTION_TYPE_BOOL;
        case KALOS_TOKEN_NUMBER: return FUNCTION_TYPE_NUMBER;
        case KALOS_TOKEN_ANY: return FUNCTION_TYPE_ANY;
        default: THROW(ERROR_UNEXPECTED_TOKEN);
    }
    TRY_EXIT;
    return FUNCTION_TYPE_VOID;
}

void parse_idl_type(struct parse_state* parse_state) {
    kalos_token peek, token;
    TRY(peek = lex_peek(parse_state));
    kalos_function_type type = FUNCTION_TYPE_VOID;
    if (peek == KALOS_TOKEN_COLON) {
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_COLON));
        TRY(token = lex(parse_state));
        TRY(type = parse_map_type(parse_state, token));
    }
    TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, type));
    TRY_EXIT;
}

void parse_idl_args(struct parse_state* parse_state) {
    kalos_token peek;
    TRY(peek = lex_peek(parse_state));
    int args = 0;
    if (peek == KALOS_TOKEN_PAREN_OPEN) {
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_PAREN_OPEN));
        TRY(peek = lex_peek(parse_state));
        if (peek != KALOS_TOKEN_PAREN_CLOSE) {
            loop {
                args++;
                TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
                TRY(parse_push_token(parse_state));
                TRY(parse_idl_type(parse_state));
                TRY(peek = lex_peek(parse_state));
                if (peek == KALOS_TOKEN_ELLIPSIS) {
                    // Varargs must be the last arg
                    TRY(parse_assert_token(parse_state, KALOS_TOKEN_ELLIPSIS));
                    TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, 1));
                    TRY(parse_make_list(parse_state, 3));
                    break;
                } else if (peek == KALOS_TOKEN_PAREN_CLOSE) {
                    TRY(parse_make_list(parse_state, 2));
                    break;
                }
                TRY(parse_make_list(parse_state, 2));
                TRY(parse_assert_token(parse_state, KALOS_TOKEN_COMMA));
            }
        }
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_PAREN_CLOSE));
    }
    TRY(parse_make_list(parse_state, args));

    TRY_EXIT;
}

void parse_idl_binding(struct parse_state* parse_state, kalos_int invoke_id) {
    kalos_token peek, token;
    TRY(peek = lex_peek(parse_state));
    if (peek == KALOS_TOKEN_C) {
        TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, KALOS_TOKEN_C));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_C));
        TRY(token = lex(parse_state));
        TRY(parse_string_expression(parse_state, token));
    } else {
        TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, 0));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
        TRY(parse_push_token(parse_state));
    }
    TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, invoke_id));
    TRY(parse_make_list(parse_state, 3));
    TRY_EXIT;
}

void parse_idl_prop(struct parse_state* parse_state) {
    kalos_token peek;
    TRY(parse_idl_type(parse_state));
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_BRA_OPEN));
    TRY(peek = lex_peek(parse_state));
    uint16_t invoke_id = ++parse_state->function_invoke_id;
    if (peek == KALOS_TOKEN_READ) {
        TRY(parse_assert_token(parse_state, peek));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_EQ));
        TRY(parse_idl_binding(parse_state, invoke_id));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
        TRY(peek = lex_peek(parse_state));
    } else if (peek == KALOS_TOKEN_WRITE) {
        TRY(parse_make_list(parse_state, 0));
    } else {
        THROW(ERROR_UNEXPECTED_TOKEN);
    }
    if (peek == KALOS_TOKEN_WRITE) {
        TRY(parse_assert_token(parse_state, peek));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_EQ));
        TRY(parse_idl_binding(parse_state, invoke_id));
        TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
        TRY(peek = lex_peek(parse_state));
    } else {
        TRY(parse_make_list(parse_state, 0));
    }
    TRY(parse_make_list(parse_state, 5));
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_BRA_CLOSE));
    TRY_EXIT;
}

void parse_idl_fn(struct parse_state* parse_state) {
    TRY(parse_idl_args(parse_state));
    TRY(parse_idl_type(parse_state));
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_EQ));
    TRY(parse_idl_binding(parse_state, parse_state->function_invoke_id));
    TRY(parse_make_list(parse_state, 3));
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
    TRY_EXIT;
}

void parse_idl_block(struct parse_state* parse_state) {
    TRY(write_next_handler_section(parse_state, KALOS_IDL_HANDLER_ADDRESS));
    TRY(parse_assert_token(parse_state, KALOS_TOKEN_BRA_OPEN));
    kalos_token token, peek;
    int idl_count = 0;
    int module_index = 0;
    loop {
        TRY(token = lex(parse_state));
        if (token == KALOS_TOKEN_BRA_CLOSE) {
            break;
        }
        TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, token));
        if (token == KALOS_TOKEN_MODULE) {
            parse_state->function_invoke_id = 0;
            int hander_id = 0;
            TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, module_index++));
            TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
            TRY(parse_push_token(parse_state));
            TRY(parse_assert_token(parse_state, KALOS_TOKEN_BRA_OPEN));
            int export_count = 0;
            loop {
                TRY(token = lex(parse_state));
                if (token == KALOS_TOKEN_BRA_CLOSE) {
                    break;
                }
                TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, token));
                TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
                TRY(parse_push_token(parse_state));
                if (token == KALOS_TOKEN_FN) {
                    TRY(peek = lex_peek(parse_state));
                    parse_state->function_invoke_id++;
                    if (peek == KALOS_TOKEN_BRA_OPEN) {
                        int overload_count = 0;
                        TRY(parse_assert_token(parse_state, KALOS_TOKEN_BRA_OPEN));
                        loop {
                            overload_count++;
                            TRY(parse_idl_fn(parse_state));
                            TRY(peek = lex_peek(parse_state));
                            if (peek == KALOS_TOKEN_BRA_CLOSE) {
                                TRY(parse_assert_token(parse_state, KALOS_TOKEN_BRA_CLOSE));
                                break;
                            }
                        }
                        TRY(parse_make_list(parse_state, overload_count));
                    } else {
                        TRY(parse_idl_fn(parse_state));
                        TRY(parse_make_list(parse_state, 1));
                    }
                    TRY(parse_make_list(parse_state, 3));
                } else if (token == KALOS_TOKEN_PROP) {
                    TRY(parse_idl_prop(parse_state));
                } else if (token == KALOS_TOKEN_OBJECT) {
                    TRY(parse_assert_token(parse_state, KALOS_TOKEN_BRA_OPEN));
                    int prop_count = 0;
                    loop {
                        TRY(peek = lex_peek(parse_state));
                        if (peek == KALOS_TOKEN_BRA_CLOSE) {
                            break;
                        }
                        TRY(parse_assert_token(parse_state, KALOS_TOKEN_PROP));
                        TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, token));
                        TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
                        TRY(parse_push_token(parse_state));
                        prop_count++;
                        TRY(parse_idl_prop(parse_state));
                    }
                    TRY(parse_make_list(parse_state, prop_count));
                    TRY(parse_assert_token(parse_state, KALOS_TOKEN_BRA_CLOSE));
                    TRY(parse_make_list(parse_state, 3));
                } else if (token == KALOS_TOKEN_HANDLER) {
                    TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, ++hander_id));
                    TRY(parse_idl_args(parse_state));
                    TRY(parse_idl_type(parse_state));
                    TRY(parse_make_list(parse_state, 5));
                    TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
                } else if (token == KALOS_TOKEN_CONST) {
                    TRY(parse_idl_type(parse_state));
                    TRY(parse_assert_token(parse_state, KALOS_TOKEN_EQ));
                    TRY(parse_expression(parse_state));
                    TRY(parse_make_list(parse_state, 4));
                    TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
                } else {
                    THROW(ERROR_UNEXPECTED_TOKEN);
                }
                export_count++;
            }
            TRY(parse_make_list(parse_state, export_count));
            TRY(parse_make_list(parse_state, 4));
        } else if (token == KALOS_TOKEN_PREFIX) {
            TRY(token = lex(parse_state));
            TRY(parse_string_expression(parse_state, token));
            TRY(parse_make_list(parse_state, 2));
            TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
        } else if (token == KALOS_TOKEN_DISPATCH) {
            TRY(peek = lex_peek(parse_state));
            if (peek != KALOS_TOKEN_NAME && peek != KALOS_TOKEN_INTERNAL) {
                THROW(ERROR_UNEXPECTED_TOKEN);
            }
            TRY(parse_push_op_1(parse_state, KALOS_OP_PUSH_INTEGER, peek));
            TRY(parse_assert_token(parse_state, peek));
            TRY(parse_make_list(parse_state, 2));
            TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
        } else {
            THROW(ERROR_UNEXPECTED_TOKEN);
        }
        idl_count++;
    }
    TRY(parse_make_list(parse_state, idl_count));
    TRY(parse_push_op(parse_state, KALOS_OP_IDL));
    TRY(parse_push_op(parse_state, KALOS_OP_END));
    TRY_EXIT;
}

void parse_file(struct parse_state* parse_state, const char kalos_far* s) {
    kalos_lex_init(s, &parse_state->lex_state);
    bool code_phase = false;
    bool found_idl = false;
    bool export = false;
    bool has_written_global_section = false;
    loop {
        kalos_token token, peek;
        TRY(token = lex(parse_state));
        if (token == KALOS_TOKEN_EOF) {
            if (!code_phase && !found_idl) {
                THROW(ERROR_MISSING_HANDLE);
            }
            break;
        }

        if (code_phase && token != KALOS_TOKEN_ON && token != KALOS_TOKEN_FN && token != KALOS_TOKEN_EXPORT) {
            THROW(ERROR_UNEXPECTED_TOKEN);
        }

        if (token == KALOS_TOKEN_IMPORT) {
            TRY(peek = lex_peek(parse_state));
            if (peek == KALOS_TOKEN_PERIOD) {
                // Relative import, only current directory supported
                TRY(parse_assert_token(parse_state, KALOS_TOKEN_PERIOD));
                TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
                if (!parse_state->options.loader) {
                    THROW(ERROR_NO_LOADER);
                }
                kalos_load_result result;
                LOG("Loading module %s", parse_state->token);
                kalos_loaded_script script = parse_state->options.loader(parse_state->state, parse_state->token, &result);
                if (result == SCRIPT_LOAD_ERROR) {
                    THROW(ERROR_INVALID_IMPORT);
                }
                kalos_lex_state old_lex_state = parse_state->lex_state;
                LOG("Loaded module %s, parsing", parse_state->token);
                TRY(parse_file(parse_state, script.text));
                LOG("Parsed module %s", parse_state->token);
                if (parse_state->options.unloader) {
                    parse_state->options.unloader(parse_state->state, script);
                }
                parse_state->lex_state = old_lex_state;
            } else {
                TRY(parse_assert_token(parse_state, KALOS_TOKEN_WORD));
                struct name_resolution_result res;
                TRY(res = resolve_word(parse_state, NULL));
                if (res.type == NAME_RESOLUTION_UNIMPORTED_MODULE) {
                    parse_state->imported_modules[parse_state->module_index++] = res.module;
                } else if (res.type == NAME_RESOLUTION_MODULE) {
                    THROW(ERROR_DUPLICATE_IMPORT);
                } else {
                    THROW(ERROR_INVALID_IMPORT);
                }
            }

            TRY(parse_assert_token(parse_state, KALOS_TOKEN_SEMI));
        } else if (token == KALOS_TOKEN_EXPORT) {
            TRY(peek = lex_peek(parse_state));
            if (peek != KALOS_TOKEN_VAR && peek != KALOS_TOKEN_CONST && peek != KALOS_TOKEN_FN) {
                THROW(ERROR_UNEXPECTED_EXPORT_TYPE);
            }
            export = true;
        } else if (token == KALOS_TOKEN_VAR || token == KALOS_TOKEN_CONST) {
            if (!has_written_global_section) {
                TRY(write_next_handler_section(parse_state, KALOS_GLOBAL_HANDLER_ADDRESS));
                has_written_global_section = true;
            }
            TRY(parse_var_statement(parse_state, &parse_state->globals, export));
            export = false;
        } else if (token == KALOS_TOKEN_ON || token == KALOS_TOKEN_FN) {
            if (!code_phase) {
                code_phase = true;
                // TODO: If we don't write this global section, many of the tests change
                if (!has_written_global_section) {
                    TRY(write_next_handler_section(parse_state, KALOS_GLOBAL_HANDLER_ADDRESS));
                }
                TRY(parse_push_op(parse_state, KALOS_OP_END));
                has_written_global_section = false;
            }
            memset(&parse_state->locals, 0, sizeof(parse_state->locals));
            TRY(parse_function_statement(parse_state, export));
            if (parse_state->last_section_header) {
                parse_state->last_section_header->locals_size = parse_state->locals.var_index;
            }
            export = false;
        } else if (token == KALOS_TOKEN_IDL) {
            found_idl = true;
            // TODO: If we don't write this global section, many of the tests change
            if (!has_written_global_section) {
                TRY(write_next_handler_section(parse_state, KALOS_GLOBAL_HANDLER_ADDRESS));
            }
            TRY(parse_push_op(parse_state, KALOS_OP_END));
            has_written_global_section = false;
            TRY(parse_idl_block(parse_state));
        } else {
            THROW(ERROR_UNEXPECTED_TOKEN);
        }
    }

    // In case we have a var-only module
    if (has_written_global_section) {
        TRY(parse_push_op(parse_state, KALOS_OP_END));
    }

    TRY_EXIT;
}

kalos_parse_result kalos_parse(const char kalos_far* s, kalos_module_parsed modules, kalos_parse_options options,
    kalos_state* state, kalos_script script, size_t script_size) {
    struct parse_state parse_state_data = {
        .state = state,
        .options = options,
        .output_script = script,
    };
    if (modules.buffer) {
        parse_state_data.all_modules = modules;
        parse_state_data.extra_builtins = kalos_module_find_module(modules, "builtin");
        parse_state_data.dispatch_name = (kalos_module_get_header(modules)->flags & KALOS_MODULE_FLAG_DISPATCH_NAME) != 0;
    }
    kalos_script_header* header = NULL;
    if (script) {
        header = (kalos_script_header*)parse_state_data.output_script;
        memset(header, 0, sizeof(*header));
        header->signature[0] = 'K';
        header->signature[1] = 'L';
        header->signature[2] = 26; // ^Z
        header->signature[3] = 0;
    }
    parse_state_data.output_script_index = sizeof(*header);
    
    struct parse_state* parse_state = &parse_state_data;
    TRY(parse_file(parse_state, s));

    if (header) {
        header->globals_size = parse_state->globals.var_index;
        header->length = parse_state->output_script_index;
    }
    kalos_parse_result res = {0};
    res.success = true;
    res.size = parse_state->output_script_index;
    return res;

    TRY_EXIT;
    res.success = false;
    res.error = parse_state->failure_message;
    res.line = parse_state->lex_state.line;
    return res;
}

kalos_parse_result kalos_parse_buffer(const char kalos_far* script_text,
    kalos_module_parsed modules, kalos_parse_options options,
    kalos_state* state, kalos_buffer* buffer) {
    kalos_buffer output = {0};
    *buffer = output;
    kalos_parse_result result = kalos_parse(script_text, modules, options, state, NULL, 0);
    if (result.error) {
        return result;
    }
    output = kalos_buffer_alloc(state, result.size);
    result = kalos_parse(script_text, modules, options, state, output.buffer, result.size);
    if (result.error) {
        kalos_buffer_free(output);
        return result;
    }
    *buffer = output;
    return result;
}
