#pragma once

#include "kalos.h"

#ifdef __WATCOMC__
#define kalos_lex_strncpy _fstrncpy
#else
#define kalos_lex_strncpy strncpy
#endif

#define KALOS_TOKEN(x) KALOS_TOKEN_##x,
typedef enum kalos_token {
    #include "_kalos_constants.inc"
    KALOS_TOKEN_MAX,
} kalos_token;

extern const char* kalos_token_strings[KALOS_TOKEN_MAX + 1];

typedef struct kalos_lex_state {
    const char kalos_far* s;
    int line;
    int string_interp_state;
    int string_interp_mode;
} kalos_lex_state;

void kalos_lex_init(const char kalos_far* s, kalos_lex_state* state);
kalos_token kalos_lex(kalos_lex_state* state, char* output_buffer);
kalos_token kalos_lex_peek(kalos_lex_state* state, char* output_buffer);
