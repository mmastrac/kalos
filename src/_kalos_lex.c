/* Generated by re2c 2.1.1 on Sun Jun 20 16:17:01 2021 */
#include <string.h>
#include <stdlib.h>
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

    const char kalos_far *yyt1; 
    for (;;) {
    
{
	char yych;
	unsigned int yyaccept = 0;
	switch (c) {
	case yycinit:
		goto yyc_init;
	case yycinstring:
		goto yyc_instring;
	case yycinstringmulti:
		goto yyc_instringmulti;
	case yycstringformat:
		goto yyc_stringformat;
	case yyccomment:
		goto yyc_comment;
	}
/* *********************************** */
yyc_init:
	yych = *(*s);
	switch (yych) {
	case 0x00:	goto yy2;
	case '\t':
	case '\r':
	case ' ':	goto yy6;
	case '\n':	goto yy9;
	case '!':	goto yy11;
	case '"':	goto yy13;
	case '#':	goto yy15;
	case '%':	goto yy17;
	case '&':	goto yy19;
	case '(':	goto yy21;
	case ')':	goto yy23;
	case '*':	goto yy25;
	case '+':	goto yy27;
	case ',':	goto yy29;
	case '-':	goto yy31;
	case '.':	goto yy33;
	case '/':	goto yy35;
	case '0':
		yyt1 = (*s);
		goto yy37;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		yyt1 = (*s);
		goto yy39;
	case ':':	goto yy41;
	case ';':	goto yy43;
	case '<':	goto yy45;
	case '=':	goto yy47;
	case '>':	goto yy49;
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'g':
	case 'j':
	case 'k':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 's':
	case 'u':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
		yyt1 = (*s);
		goto yy51;
	case '[':	goto yy54;
	case ']':	goto yy56;
	case '^':	goto yy58;
	case 'b':
		yyt1 = (*s);
		goto yy60;
	case 'c':
		yyt1 = (*s);
		goto yy61;
	case 'd':
		yyt1 = (*s);
		goto yy62;
	case 'e':
		yyt1 = (*s);
		goto yy63;
	case 'f':
		yyt1 = (*s);
		goto yy64;
	case 'h':
		yyt1 = (*s);
		goto yy65;
	case 'i':
		yyt1 = (*s);
		goto yy66;
	case 'l':
		yyt1 = (*s);
		goto yy67;
	case 'r':
		yyt1 = (*s);
		goto yy68;
	case 't':
		yyt1 = (*s);
		goto yy69;
	case 'v':
		yyt1 = (*s);
		goto yy70;
	case '{':	goto yy71;
	case '|':	goto yy73;
	case '}':	goto yy75;
	case '~':	goto yy77;
	default:	goto yy4;
	}
yy2:
	++(*s);
	{ return KALOS_TOKEN_EOF; }
yy4:
	++(*s);
	{ return KALOS_TOKEN_ERROR; }
yy6:
	yych = *++(*s);
	switch (yych) {
	case '\t':
	case '\r':
	case ' ':	goto yy6;
	default:	goto yy8;
	}
yy8:
	{ continue; }
yy9:
	++(*s);
	{ if (state->string_interp_state != STRING_STATE_NORMAL) return KALOS_TOKEN_ERROR; (*line)++; continue; }
yy11:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy79;
	default:	goto yy12;
	}
yy12:
	{ return KALOS_TOKEN_BANG; }
yy13:
	yyaccept = 0;
	yych = *(YYMARKER = ++(*s));
	switch (yych) {
	case '"':	goto yy81;
	default:	goto yy14;
	}
yy14:
	c = yycinstring;
	goto yyc_instring;
yy15:
	++(*s);
	c = yyccomment;
	goto yyc_comment;
yy17:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy83;
	default:	goto yy18;
	}
yy18:
	{ return KALOS_TOKEN_PERCENT; }
yy19:
	yych = *++(*s);
	switch (yych) {
	case '&':	goto yy85;
	case '=':	goto yy87;
	default:	goto yy20;
	}
yy20:
	{ return KALOS_TOKEN_BITAND; }
yy21:
	++(*s);
	{ return KALOS_TOKEN_PAREN_OPEN; }
yy23:
	++(*s);
	{ return KALOS_TOKEN_PAREN_CLOSE; }
yy25:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy89;
	default:	goto yy26;
	}
yy26:
	{ return KALOS_TOKEN_STAR; }
yy27:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy91;
	default:	goto yy28;
	}
yy28:
	{ return KALOS_TOKEN_PLUS; }
yy29:
	++(*s);
	{ return KALOS_TOKEN_COMMA; }
yy31:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy93;
	default:	goto yy32;
	}
yy32:
	{ return KALOS_TOKEN_MINUS; }
yy33:
	++(*s);
	{ return KALOS_TOKEN_PERIOD; }
yy35:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy95;
	default:	goto yy36;
	}
yy36:
	{ return KALOS_TOKEN_SLASH; }
yy37:
	yyaccept = 1;
	yych = *(YYMARKER = ++(*s));
	switch (yych) {
	case 'X':
	case 'x':	goto yy97;
	default:	goto yy38;
	}
yy38:
	word_start = yyt1;
	word_end = (*s);
	{ kalos_lex_strncpy(output, word_start, word_end - word_start); output[word_end - word_start] = 0; return KALOS_TOKEN_INTEGER; }
yy39:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy39;
	default:	goto yy38;
	}
yy41:
	++(*s);
	{
            if (state->string_interp_state == STRING_STATE_EXPR) {
                c = yycstringformat;
                continue;
            }
            return KALOS_TOKEN_COLON;
        }
yy43:
	++(*s);
	{ return KALOS_TOKEN_SEMI; }
yy45:
	yych = *++(*s);
	switch (yych) {
	case '<':	goto yy98;
	case '=':	goto yy100;
	default:	goto yy46;
	}
yy46:
	{ return KALOS_TOKEN_LT; }
yy47:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy102;
	default:	goto yy48;
	}
yy48:
	{ return KALOS_TOKEN_EQ; }
yy49:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy104;
	case '>':	goto yy106;
	default:	goto yy50;
	}
yy50:
	{ return KALOS_TOKEN_GT; }
yy51:
	yych = *++(*s);
yy52:
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy53;
	}
yy53:
	word_start = yyt1;
	word_end = (*s);
	{ kalos_lex_strncpy(output, word_start, word_end - word_start); output[word_end - word_start] = 0; return KALOS_TOKEN_WORD; }
yy54:
	++(*s);
	{ return KALOS_TOKEN_SQBRA_OPEN; }
yy56:
	++(*s);
	{ return KALOS_TOKEN_SQBRA_CLOSE; }
yy58:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy108;
	default:	goto yy59;
	}
yy59:
	{ return KALOS_TOKEN_BITXOR; }
yy60:
	yych = *++(*s);
	switch (yych) {
	case 'r':	goto yy110;
	default:	goto yy52;
	}
yy61:
	yych = *++(*s);
	switch (yych) {
	case 'o':	goto yy111;
	default:	goto yy52;
	}
yy62:
	yych = *++(*s);
	switch (yych) {
	case 'e':	goto yy112;
	default:	goto yy52;
	}
yy63:
	yych = *++(*s);
	switch (yych) {
	case 'l':	goto yy113;
	default:	goto yy52;
	}
yy64:
	yych = *++(*s);
	switch (yych) {
	case 'a':	goto yy114;
	case 'o':	goto yy115;
	default:	goto yy52;
	}
yy65:
	yych = *++(*s);
	switch (yych) {
	case 'a':	goto yy116;
	default:	goto yy52;
	}
yy66:
	yych = *++(*s);
	switch (yych) {
	case 'f':	goto yy117;
	case 'm':	goto yy119;
	case 'n':	goto yy120;
	default:	goto yy52;
	}
yy67:
	yych = *++(*s);
	switch (yych) {
	case 'o':	goto yy122;
	default:	goto yy52;
	}
yy68:
	yych = *++(*s);
	switch (yych) {
	case 'e':	goto yy123;
	default:	goto yy52;
	}
yy69:
	yych = *++(*s);
	switch (yych) {
	case 'r':	goto yy124;
	default:	goto yy52;
	}
yy70:
	yych = *++(*s);
	switch (yych) {
	case 'a':	goto yy125;
	default:	goto yy52;
	}
yy71:
	++(*s);
	{ return KALOS_TOKEN_BRA_OPEN; }
yy73:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy126;
	case '|':	goto yy128;
	default:	goto yy74;
	}
yy74:
	{ return KALOS_TOKEN_BITOR; }
yy75:
	++(*s);
	{
            if (state->string_interp_state == STRING_STATE_EXPR) { 
                state->string_interp_state = STRING_STATE_DONE;
                output[0] = 0; // empty format spec
                return KALOS_TOKEN_STRING_FORMAT_SPEC;
            }
            return KALOS_TOKEN_BRA_CLOSE; 
        }
yy77:
	++(*s);
	{ return KALOS_TOKEN_TILDE; }
yy79:
	++(*s);
	{ return KALOS_TOKEN_NOTEQ; }
yy81:
	yych = *++(*s);
	switch (yych) {
	case '"':	goto yy130;
	default:	goto yy82;
	}
yy82:
	(*s) = YYMARKER;
	if (yyaccept == 0) {
		goto yy14;
	} else {
		goto yy38;
	}
yy83:
	++(*s);
	{ return KALOS_TOKEN_EQ_PERCENT; }
yy85:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy132;
	default:	goto yy86;
	}
yy86:
	{ return KALOS_TOKEN_LOGAND; }
yy87:
	++(*s);
	{ return KALOS_TOKEN_EQ_BITAND; }
yy89:
	++(*s);
	{ return KALOS_TOKEN_EQ_STAR; }
yy91:
	++(*s);
	{ return KALOS_TOKEN_EQ_PLUS; }
yy93:
	++(*s);
	{ return KALOS_TOKEN_EQ_MINUS; }
yy95:
	++(*s);
	{ return KALOS_TOKEN_EQ_SLASH; }
yy97:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':	goto yy134;
	default:	goto yy82;
	}
yy98:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy136;
	default:	goto yy99;
	}
yy99:
	{ return KALOS_TOKEN_LEFT_SHIFT; }
yy100:
	++(*s);
	{ return KALOS_TOKEN_LTE; }
yy102:
	++(*s);
	{ return KALOS_TOKEN_EQEQ; }
yy104:
	++(*s);
	{ return KALOS_TOKEN_GTE; }
yy106:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy138;
	default:	goto yy107;
	}
yy107:
	{ return KALOS_TOKEN_RIGHT_SHIFT; }
yy108:
	++(*s);
	{ return KALOS_TOKEN_EQ_BITXOR; }
yy110:
	yych = *++(*s);
	switch (yych) {
	case 'e':	goto yy140;
	default:	goto yy52;
	}
yy111:
	yych = *++(*s);
	switch (yych) {
	case 'n':	goto yy141;
	default:	goto yy52;
	}
yy112:
	yych = *++(*s);
	switch (yych) {
	case 'b':	goto yy142;
	default:	goto yy52;
	}
yy113:
	yych = *++(*s);
	switch (yych) {
	case 's':	goto yy143;
	default:	goto yy52;
	}
yy114:
	yych = *++(*s);
	switch (yych) {
	case 'l':	goto yy144;
	default:	goto yy52;
	}
yy115:
	yych = *++(*s);
	switch (yych) {
	case 'r':	goto yy145;
	default:	goto yy52;
	}
yy116:
	yych = *++(*s);
	switch (yych) {
	case 'n':	goto yy147;
	default:	goto yy52;
	}
yy117:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy118;
	}
yy118:
	{ return KALOS_TOKEN_IF; }
yy119:
	yych = *++(*s);
	switch (yych) {
	case 'p':	goto yy148;
	default:	goto yy52;
	}
yy120:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy121;
	}
yy121:
	{ return KALOS_TOKEN_IN; }
yy122:
	yych = *++(*s);
	switch (yych) {
	case 'o':	goto yy149;
	default:	goto yy52;
	}
yy123:
	yych = *++(*s);
	switch (yych) {
	case 't':	goto yy150;
	default:	goto yy52;
	}
yy124:
	yych = *++(*s);
	switch (yych) {
	case 'u':	goto yy151;
	default:	goto yy52;
	}
yy125:
	yych = *++(*s);
	switch (yych) {
	case 'r':	goto yy152;
	default:	goto yy52;
	}
yy126:
	++(*s);
	{ return KALOS_TOKEN_EQ_BITOR; }
yy128:
	yych = *++(*s);
	switch (yych) {
	case '=':	goto yy154;
	default:	goto yy129;
	}
yy129:
	{ return KALOS_TOKEN_LOGOR; }
yy130:
	++(*s);
	c = yycinstringmulti;
	goto yyc_instringmulti;
yy132:
	++(*s);
	{ return KALOS_TOKEN_EQ_LOGAND; }
yy134:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':	goto yy134;
	default:	goto yy38;
	}
yy136:
	++(*s);
	{ return KALOS_TOKEN_EQ_LEFT_SHIFT; }
yy138:
	++(*s);
	{ return KALOS_TOKEN_EQ_RIGHT_SHIFT; }
yy140:
	yych = *++(*s);
	switch (yych) {
	case 'a':	goto yy156;
	default:	goto yy52;
	}
yy141:
	yych = *++(*s);
	switch (yych) {
	case 's':	goto yy157;
	case 't':	goto yy158;
	default:	goto yy52;
	}
yy142:
	yych = *++(*s);
	switch (yych) {
	case 'u':	goto yy159;
	default:	goto yy52;
	}
yy143:
	yych = *++(*s);
	switch (yych) {
	case 'e':	goto yy160;
	default:	goto yy52;
	}
yy144:
	yych = *++(*s);
	switch (yych) {
	case 's':	goto yy162;
	default:	goto yy52;
	}
yy145:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy146;
	}
yy146:
	{ return KALOS_TOKEN_FOR; }
yy147:
	yych = *++(*s);
	switch (yych) {
	case 'd':	goto yy163;
	default:	goto yy52;
	}
yy148:
	yych = *++(*s);
	switch (yych) {
	case 'o':	goto yy164;
	default:	goto yy52;
	}
yy149:
	yych = *++(*s);
	switch (yych) {
	case 'p':	goto yy165;
	default:	goto yy52;
	}
yy150:
	yych = *++(*s);
	switch (yych) {
	case 'u':	goto yy167;
	default:	goto yy52;
	}
yy151:
	yych = *++(*s);
	switch (yych) {
	case 'e':	goto yy168;
	default:	goto yy52;
	}
yy152:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy153;
	}
yy153:
	{ return KALOS_TOKEN_VAR; }
yy154:
	++(*s);
	{ return KALOS_TOKEN_EQ_LOGOR; }
yy156:
	yych = *++(*s);
	switch (yych) {
	case 'k':	goto yy170;
	default:	goto yy52;
	}
yy157:
	yych = *++(*s);
	switch (yych) {
	case 't':	goto yy172;
	default:	goto yy52;
	}
yy158:
	yych = *++(*s);
	switch (yych) {
	case 'i':	goto yy174;
	default:	goto yy52;
	}
yy159:
	yych = *++(*s);
	switch (yych) {
	case 'g':	goto yy175;
	default:	goto yy52;
	}
yy160:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy161;
	}
yy161:
	{ return KALOS_TOKEN_ELSE; }
yy162:
	yych = *++(*s);
	switch (yych) {
	case 'e':	goto yy176;
	default:	goto yy52;
	}
yy163:
	yych = *++(*s);
	switch (yych) {
	case 'l':	goto yy178;
	default:	goto yy52;
	}
yy164:
	yych = *++(*s);
	switch (yych) {
	case 'r':	goto yy179;
	default:	goto yy52;
	}
yy165:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy166;
	}
yy166:
	{ return KALOS_TOKEN_LOOP; }
yy167:
	yych = *++(*s);
	switch (yych) {
	case 'r':	goto yy180;
	default:	goto yy52;
	}
yy168:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy169;
	}
yy169:
	{ return KALOS_TOKEN_TRUE; }
yy170:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy171;
	}
yy171:
	{ return KALOS_TOKEN_BREAK; }
yy172:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy173;
	}
yy173:
	{ return KALOS_TOKEN_CONST; }
yy174:
	yych = *++(*s);
	switch (yych) {
	case 'n':	goto yy181;
	default:	goto yy52;
	}
yy175:
	yych = *++(*s);
	switch (yych) {
	case 'g':	goto yy182;
	default:	goto yy52;
	}
yy176:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy177;
	}
yy177:
	{ return KALOS_TOKEN_FALSE; }
yy178:
	yych = *++(*s);
	switch (yych) {
	case 'e':	goto yy183;
	default:	goto yy52;
	}
yy179:
	yych = *++(*s);
	switch (yych) {
	case 't':	goto yy185;
	default:	goto yy52;
	}
yy180:
	yych = *++(*s);
	switch (yych) {
	case 'n':	goto yy187;
	default:	goto yy52;
	}
yy181:
	yych = *++(*s);
	switch (yych) {
	case 'u':	goto yy189;
	default:	goto yy52;
	}
yy182:
	yych = *++(*s);
	switch (yych) {
	case 'e':	goto yy190;
	default:	goto yy52;
	}
yy183:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy184;
	}
yy184:
	{ return KALOS_TOKEN_HANDLE; }
yy185:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy186;
	}
yy186:
	{ return KALOS_TOKEN_IMPORT; }
yy187:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy188;
	}
yy188:
	{ return KALOS_TOKEN_RETURN; }
yy189:
	yych = *++(*s);
	switch (yych) {
	case 'e':	goto yy191;
	default:	goto yy52;
	}
yy190:
	yych = *++(*s);
	switch (yych) {
	case 'r':	goto yy193;
	default:	goto yy52;
	}
yy191:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy192;
	}
yy192:
	{ return KALOS_TOKEN_CONTINUE; }
yy193:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy51;
	default:	goto yy194;
	}
yy194:
	{ return KALOS_TOKEN_DEBUGGER; }
/* *********************************** */
yyc_instring:
	yych = *(*s);
	switch (yych) {
	case 0x00:	goto yy197;
	case '\n':
	case '\r':	goto yy201;
	case '"':	goto yy203;
	case '\\':	goto yy205;
	case '{':	goto yy206;
	case '}':	goto yy208;
	default:	goto yy199;
	}
yy197:
	++(*s);
	{ return KALOS_TOKEN_ERROR; }
yy199:
	++(*s);
yy200:
	ch = (*s) - 1;
	{ *output++ = *ch; continue; }
yy201:
	++(*s);
	{ return KALOS_TOKEN_ERROR; }
yy203:
	++(*s);
	c = yycinit;
	{ *output++ = 0; return KALOS_TOKEN_STRING; }
yy205:
	yych = *++(*s);
	switch (yych) {
	case '"':	goto yy209;
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'o':
	case 'p':
	case 'q':
	case 's':
	case 'u':
	case 'v':
	case 'w':
	case 'y':
	case 'z':	goto yy211;
	case 'n':	goto yy213;
	case 'r':	goto yy215;
	case 't':	goto yy217;
	case 'x':	goto yy219;
	default:	goto yy200;
	}
yy206:
	yych = *++(*s);
	switch (yych) {
	case '{':	goto yy220;
	default:	goto yy207;
	}
yy207:
	{
            state->string_interp_state = STRING_STATE_EXPR;
            state->string_interp_mode = c;
            *output++ = 0;
            return KALOS_TOKEN_STRING_INTERPOLATION;
        }
yy208:
	yych = *++(*s);
	switch (yych) {
	case '}':	goto yy222;
	default:	goto yy200;
	}
yy209:
	++(*s);
	{ *output++ = '"'; continue; }
yy211:
	++(*s);
yy212:
	{ return KALOS_TOKEN_ERROR; }
yy213:
	++(*s);
	{ *output++ = '\n'; continue; }
yy215:
	++(*s);
	{ *output++ = '\r'; continue; }
yy217:
	++(*s);
	{ *output++ = '\t'; continue; }
yy219:
	yych = *(YYMARKER = ++(*s));
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':	goto yy224;
	default:	goto yy212;
	}
yy220:
	++(*s);
	{ *output++ = '{'; continue; }
yy222:
	++(*s);
	{ *output++ = '}'; continue; }
yy224:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':	goto yy226;
	default:	goto yy225;
	}
yy225:
	(*s) = YYMARKER;
	goto yy212;
yy226:
	++(*s);
	ch = (*s) - 2;
	{ char n[3] = { ch[0], ch[1], 0 }; *output++ = (char)strtol(n, NULL, 16); continue; }
/* *********************************** */
yyc_instringmulti:
	yych = *(*s);
	switch (yych) {
	case 0x00:	goto yy230;
	case '\n':
	case '\r':	goto yy234;
	case '"':	goto yy236;
	case '\\':	goto yy237;
	case '{':	goto yy238;
	case '}':	goto yy240;
	default:	goto yy232;
	}
yy230:
	++(*s);
	{ return KALOS_TOKEN_ERROR; }
yy232:
	++(*s);
yy233:
	ch = (*s) - 1;
	{ *output++ = *ch; continue; }
yy234:
	++(*s);
	ch = (*s) - 1;
	{ *output++ = *ch; continue; }
yy236:
	yych = *++(*s);
	switch (yych) {
	case '"':	goto yy243;
	default:	goto yy241;
	}
yy237:
	yych = *++(*s);
	switch (yych) {
	case '"':	goto yy244;
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'o':
	case 'p':
	case 'q':
	case 's':
	case 'u':
	case 'v':
	case 'w':
	case 'y':
	case 'z':	goto yy246;
	case 'n':	goto yy248;
	case 'r':	goto yy250;
	case 't':	goto yy252;
	case 'x':	goto yy254;
	default:	goto yy233;
	}
yy238:
	yych = *++(*s);
	switch (yych) {
	case '{':	goto yy255;
	default:	goto yy239;
	}
yy239:
	{
            state->string_interp_state = STRING_STATE_EXPR;
            state->string_interp_mode = c;
            *output++ = 0;
            return KALOS_TOKEN_STRING_INTERPOLATION;
        }
yy240:
	yych = *++(*s);
	switch (yych) {
	case '}':	goto yy257;
	default:	goto yy233;
	}
yy241:
	++(*s);
	(*s) -= 1;
	ch = (*s) - 1;
	{ *output++ = '"'; continue; }
yy243:
	yych = *++(*s);
	switch (yych) {
	case '"':	goto yy261;
	default:	goto yy259;
	}
yy244:
	++(*s);
	{ *output++ = '"'; continue; }
yy246:
	++(*s);
yy247:
	{ return KALOS_TOKEN_ERROR; }
yy248:
	++(*s);
	{ *output++ = '\n'; continue; }
yy250:
	++(*s);
	{ *output++ = '\r'; continue; }
yy252:
	++(*s);
	{ *output++ = '\t'; continue; }
yy254:
	yych = *(YYMARKER = ++(*s));
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':	goto yy263;
	default:	goto yy247;
	}
yy255:
	++(*s);
	{ *output++ = '{'; continue; }
yy257:
	++(*s);
	{ *output++ = '}'; continue; }
yy259:
	++(*s);
	(*s) -= 1;
	ch = (*s) - 2;
	{ *output++ = '"'; *output++ = '"'; continue; }
yy261:
	++(*s);
	c = yycinit;
	{ *output++ = 0; return KALOS_TOKEN_STRING; }
yy263:
	yych = *++(*s);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':	goto yy265;
	default:	goto yy264;
	}
yy264:
	(*s) = YYMARKER;
	goto yy247;
yy265:
	++(*s);
	ch = (*s) - 2;
	{ char n[3] = { ch[0], ch[1], 0 }; *output++ = (char)strtol(n, NULL, 16); continue; }
/* *********************************** */
yyc_stringformat:
	yych = *(*s);
	switch (yych) {
	case 0x00:
	case '{':	goto yy269;
	case '}':
		yyt1 = (*s);
		goto yy272;
	default:
		yyt1 = (*s);
		goto yy271;
	}
yy269:
	++(*s);
yy270:
	{ return KALOS_TOKEN_ERROR; }
yy271:
	yych = *(YYMARKER = ++(*s));
	switch (yych) {
	case 0x00:
	case '{':	goto yy270;
	default:	goto yy275;
	}
yy272:
	++(*s);
	word_start = yyt1;
	word_end = (*s) - 1;
	{
            kalos_lex_strncpy(output, word_start, word_end - word_start);
            output[word_end - word_start] = 0;
            state->string_interp_state = STRING_STATE_DONE;
            return KALOS_TOKEN_STRING_FORMAT_SPEC;
        }
yy274:
	yych = *++(*s);
yy275:
	switch (yych) {
	case 0x00:
	case '{':	goto yy276;
	case '}':	goto yy272;
	default:	goto yy274;
	}
yy276:
	(*s) = YYMARKER;
	goto yy270;
/* *********************************** */
yyc_comment:
	yych = *(*s);
	switch (yych) {
	case 0x00:	goto yy279;
	case '\n':	goto yy283;
	default:	goto yy281;
	}
yy279:
	++(*s);
	{ return KALOS_TOKEN_EOF; }
yy281:
	++(*s);
	{ continue; }
yy283:
	++(*s);
	c = yycinit;
	{ (*line)++; continue; }
}

    }
}
