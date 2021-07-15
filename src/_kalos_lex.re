#include "_kalos_defines.h"
#include "_kalos_lex.h"

enum yyc_state {
    yycinit,
    yycinstring,
    yycinstringmulti,
    yyccomment,
    yycstringformat,
};

#define KALOS_TOKEN(x) #x ,
const char* kalos_token_strings[] = {
    #include "_kalos_constants.inc"
    "<invalid>",
};

enum string_state {
    STRING_STATE_NORMAL,
    STRING_STATE_EXPR,
    STRING_STATE_DONE,
};

#define KALOS_COPY_TOKEN kalos_lex_strncpy(output, word_start, word_end - word_start)

void kalos_lex_init(const char kalos_far* s, kalos_lex_state* state) {
    state->s = s;
    state->line = 1;
    state->string_interp_state = STRING_STATE_NORMAL;
    state->string_interp_mode = yycinit;
}

kalos_token kalos_lex_peek(kalos_lex_state* state, char* output) {
    kalos_lex_state temp = *state;
    return kalos_lex(&temp, output);
}

kalos_token kalos_lex(kalos_lex_state* state, char* output) {
    enum yyc_state c = yycinit;
    if (state->string_interp_state == STRING_STATE_DONE) {
        c = state->string_interp_mode;
        state->string_interp_state = STRING_STATE_NORMAL;
    }
    const char kalos_far** s = &state->s;
    int* line = &state->line;
    *output = 0;

    const char kalos_far* word_start;
    const char kalos_far* word_end;
    const char kalos_far* ch;
    const char kalos_far* YYMARKER;

    /*!stags:re2c format = "const char kalos_far *@@{tag}; "; */
    for (;;) {
        state->token_start = *s;
    /*!re2c
        re2c:api:style = free-form;
        re2c:define:YYCTYPE = char;
        re2c:define:YYCURSOR = (*s);
        re2c:yyfill:enable = 0;
        re2c:define:YYGETCONDITION = "c";
        re2c:define:YYSETCONDITION = "c = @@;";
        re2c:tags:expression = "@@{tag}";

        end  = "\x00";
        eol  = "\n";
        word = [a-zA-Z_][a-zA-Z0-9_]*;
        int  = "0" | ([1-9][0-9]*);
        hex  = "0" [xX] [0-9a-fA-F]+;
        ws   = (" " | "\t" | "\r")+;

        <init> eol { if (state->string_interp_state != STRING_STATE_NORMAL) return KALOS_TOKEN_ERROR; (*line)++; continue; }
        <init> ws { continue; }

        <init> '"""' :=> instringmulti
        <init> '"' :=> instring

        <instring,instringmulti> end { return KALOS_TOKEN_ERROR; }
        <instring,instringmulti> '\\\\' { *output++ = '\\'; continue; }
        <instring,instringmulti> '\\"' { *output++ = '"'; continue; }
        <instring,instringmulti> "\\n" { *output++ = '\n'; continue; }
        <instring,instringmulti> "\\r" { *output++ = '\r'; continue; }
        <instring,instringmulti> "\\t" { *output++ = '\t'; continue; }
        <instring,instringmulti> "\\x" @ch [a-fA-F0-9][a-fA-F0-9] { char n[3] = { ch[0], ch[1], 0 }; *output++ = (char)strtol(n, NULL, 16); continue; }
        <instring> [\n\r] { return KALOS_TOKEN_ERROR; }
        <instringmulti> @ch [\n\r] { if ((*ch) == '\n') { (*line)++; }; *output++ = *ch; continue; }
        <instring,instringmulti> "\\" [a-z] { return KALOS_TOKEN_ERROR; }
        <instring,instringmulti> "{{" { *output++ = '{'; continue; }
        <instring,instringmulti> "}}" { *output++ = '}'; continue; }
        <instring,instringmulti> "{" \ [^{] {
            state->string_interp_state = STRING_STATE_EXPR;
            state->string_interp_mode = c;
            *output++ = 0;
            return KALOS_TOKEN_STRING_INTERPOLATION;
        }
        <instring,instringmulti> @ch [^"] { *output++ = *ch; continue; }
        <instringmulti> @ch '"' / [^"] { *output++ = '"'; continue; }
        <instringmulti> @ch '""' / [^"] { *output++ = '"'; *output++ = '"'; continue; }
        <instring> '"' => init { *output++ = 0; return KALOS_TOKEN_STRING_LITERAL; }
        <instringmulti> '"""' => init { *output++ = 0; return KALOS_TOKEN_STRING_LITERAL; }

        <stringformat> * { return KALOS_TOKEN_ERROR; }
        // [[fill]align][sign][#][0][minimumwidth][.precision][type]
        <stringformat> @word_start [^\x00{}]* @word_end "}" {
            KALOS_COPY_TOKEN;
            output[word_end - word_start] = 0;
            state->string_interp_state = STRING_STATE_DONE;
            return KALOS_TOKEN_STRING_FORMAT_SPEC;
        }

        <comment> * { continue; }
        <comment> [\n] => init { (*line)++; continue; }
        <comment> end { return KALOS_TOKEN_EOF; }

        <init> * { return KALOS_TOKEN_ERROR; }
        <init> end { return KALOS_TOKEN_EOF; }
        <init> "#" :=> comment

        <init> "(" { return KALOS_TOKEN_PAREN_OPEN; }
        <init> ")" { return KALOS_TOKEN_PAREN_CLOSE; }
        <init> "{" { return KALOS_TOKEN_BRA_OPEN; }
        // This gets complicated: if we are in a string interpolation, we restart the string. If not, we
        // just emit the closing bracket token.
        <init> "}" {
            if (state->string_interp_state == STRING_STATE_EXPR) { 
                state->string_interp_state = STRING_STATE_DONE;
                output[0] = 0; // empty format spec
                return KALOS_TOKEN_STRING_FORMAT_SPEC;
            }
            return KALOS_TOKEN_BRA_CLOSE; 
        }
        <init> "[" { return KALOS_TOKEN_SQBRA_OPEN; }
        <init> "]" { return KALOS_TOKEN_SQBRA_CLOSE; }
        <init> ";" { return KALOS_TOKEN_SEMI; }
        <init> "," { return KALOS_TOKEN_COMMA; }

        <init> "=" { return KALOS_TOKEN_EQ; }
        <init> "+=" { return KALOS_TOKEN_EQ_PLUS; }
        <init> "-=" { return KALOS_TOKEN_EQ_MINUS; }
        <init> "*=" { return KALOS_TOKEN_EQ_STAR; }
        <init> "/=" { return KALOS_TOKEN_EQ_SLASH; }
        <init> "%=" { return KALOS_TOKEN_EQ_PERCENT; }
        <init> "||=" { return KALOS_TOKEN_EQ_LOGOR; }
        <init> "&&=" { return KALOS_TOKEN_EQ_LOGAND; }
        <init> "|=" { return KALOS_TOKEN_EQ_BITOR; }
        <init> "&=" { return KALOS_TOKEN_EQ_BITAND; }
        <init> "^=" { return KALOS_TOKEN_EQ_BITXOR; }
        <init> "<<=" { return KALOS_TOKEN_EQ_LEFT_SHIFT; }
        <init> ">>=" { return KALOS_TOKEN_EQ_RIGHT_SHIFT; }

        <init> "==" { return KALOS_TOKEN_EQEQ; }
        <init> "<" { return KALOS_TOKEN_LT; }
        <init> ">" { return KALOS_TOKEN_GT; }
        <init> ">=" { return KALOS_TOKEN_GTE; }
        <init> "<=" { return KALOS_TOKEN_LTE; }
        <init> "!=" { return KALOS_TOKEN_NOTEQ; }

        <init> "+" { return KALOS_TOKEN_PLUS; }
        <init> "-" { return KALOS_TOKEN_MINUS; }
        <init> "*" { return KALOS_TOKEN_STAR; }
        <init> "/" { return KALOS_TOKEN_SLASH; }
        <init> "%" { return KALOS_TOKEN_PERCENT; }
        <init> "||" { return KALOS_TOKEN_LOGOR; }
        <init> "&&" { return KALOS_TOKEN_LOGAND; }
        <init> "|" { return KALOS_TOKEN_BITOR; }
        <init> "&" { return KALOS_TOKEN_BITAND; }
        <init> "^" { return KALOS_TOKEN_BITXOR; }
        <init> "<<" { return KALOS_TOKEN_LEFT_SHIFT; }
        <init> ">>" { return KALOS_TOKEN_RIGHT_SHIFT; }

        <init> "..." { return KALOS_TOKEN_ELLIPSIS; }
        <init> "." { return KALOS_TOKEN_PERIOD; }
        <init> "!" { return KALOS_TOKEN_BANG; }
        <init> "~" { return KALOS_TOKEN_TILDE; }
        <init> ":" {
            if (state->string_interp_state == STRING_STATE_EXPR) {
                c = yycstringformat;
                continue;
            }
            return KALOS_TOKEN_COLON;
        }

        <init> "var"      { return KALOS_TOKEN_VAR; }
        <init> "const"    { return KALOS_TOKEN_CONST; }
        <init> "if"       { return KALOS_TOKEN_IF; }
        <init> "else"     { return KALOS_TOKEN_ELSE; }
        <init> "loop"     { return KALOS_TOKEN_LOOP; }
        <init> "break"    { return KALOS_TOKEN_BREAK; }
        <init> "continue" { return KALOS_TOKEN_CONTINUE; }
        <init> "debugger" { return KALOS_TOKEN_DEBUGGER; }
        <init> "return"   { return KALOS_TOKEN_RETURN; }
        <init> "for"      { return KALOS_TOKEN_FOR; }
        <init> "in"       { return KALOS_TOKEN_IN; }
        <init> "true"     { return KALOS_TOKEN_TRUE; }
        <init> "false"    { return KALOS_TOKEN_FALSE; }
        <init> "fn"       { return KALOS_TOKEN_FN; }

        <init> "idl"      { return KALOS_TOKEN_IDL; }
        <init> "on"       { return KALOS_TOKEN_ON; }
        <init> "import"   { return KALOS_TOKEN_IMPORT; }
        <init> "module"   { return KALOS_TOKEN_MODULE; }
        <init> "export"   { return KALOS_TOKEN_EXPORT; }
        <init> "from"     { return KALOS_TOKEN_FROM; }
        <init> "handler"  { return KALOS_TOKEN_HANDLER; }
        <init> "prop"     { return KALOS_TOKEN_PROP; }
        <init> "read"     { return KALOS_TOKEN_READ; }
        <init> "write"    { return KALOS_TOKEN_WRITE; }
        <init> "dispatch" { return KALOS_TOKEN_DISPATCH; }
        <init> "name"     { return KALOS_TOKEN_NAME; }
        <init> "internal" { return KALOS_TOKEN_INTERNAL; }
        <init> "prefix"   { return KALOS_TOKEN_PREFIX; }
        <init> "c"        { return KALOS_TOKEN_C; }
        <init> "object"   { return KALOS_TOKEN_OBJECT; }
        <init> "string"   { return KALOS_TOKEN_STRING; }
        <init> "void"     { return KALOS_TOKEN_VOID; }
        <init> "bool"     { return KALOS_TOKEN_BOOL; }
        <init> "number"   { return KALOS_TOKEN_NUMBER; }
        <init> "any"      { return KALOS_TOKEN_ANY; }

        <init> @word_start word @word_end { KALOS_COPY_TOKEN; output[word_end - word_start] = 0; return KALOS_TOKEN_WORD; }
        <init> @word_start (int|hex) @word_end { KALOS_COPY_TOKEN; output[word_end - word_start] = 0; return KALOS_TOKEN_INTEGER; }
    */
    }
}
