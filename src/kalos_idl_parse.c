/* Generated by re2c 2.1.1 on Tue Jun  1 15:23:59 2021 */
#include <stdbool.h>
#include <memory.h>
#include <ctype.h>
#include "kalos_idl_parse.h"
#include "kalos_module.h"

// Comparison equivalent to 'unsigned == 0'
#pragma warning 136 9

enum yyc_state {
    yycinit,
    yycmodule,
    yycfunction,
    yycfcomma,
    yycfret,
};

const char* copy_string(char* buffer, const char* a, const char* b) {
    memcpy(buffer, a, b-a);
    buffer[b-a] = 0;
    return (const char*)buffer;
}

bool kalos_idl_parse_callback(const char* s, void* context, kalos_idl_callbacks* callbacks) {
    const char* YYMARKER;
    const char *a, *b, *c, *d, *e, *f, *g, *h;
    enum yyc_state state = yycinit;

    char buffers[4][128];

    const char *yyt1; const char *yyt2; const char *yyt3; const char *yyt4; const char *yyt5; const char *yyt6; const char *yyt7; const char *yyt8; 

    for (;;) {
    
{
	char yych;
	unsigned int yyaccept = 0;
	switch (state) {
	case yycfcomma:
		goto yyc_fcomma;
	case yycfret:
		goto yyc_fret;
	case yycfunction:
		goto yyc_function;
	case yycinit:
		goto yyc_init;
	case yycmodule:
		goto yyc_module;
	}
/* *********************************** */
yyc_fcomma:
	yych = *s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy4;
	case ')':	goto yy7;
	case ',':	goto yy9;
	default:	goto yy2;
	}
yy2:
	++s;
	{ return false; }
yy4:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy4;
	default:	goto yy6;
	}
yy6:
	{ continue; }
yy7:
	++s;
	state = yycfret;
	goto yyc_fret;
yy9:
	++s;
	state = yycfunction;
	goto yyc_function;
/* *********************************** */
yyc_fret:
	yych = *s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt3 = yyt4 = NULL;
		goto yy15;
	case ':':	goto yy18;
	case '=':
		yyt3 = yyt4 = NULL;
		goto yy19;
	default:	goto yy13;
	}
yy13:
	++s;
yy14:
	{ return false; }
yy15:
	yyaccept = 0;
	yych = *(YYMARKER = ++s);
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy15;
	case '=':	goto yy20;
	default:	goto yy17;
	}
yy17:
	{ continue; }
yy18:
	yyaccept = 1;
	yych = *(YYMARKER = ++s);
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
	case 'a':
	case 'n':
	case 'o':
	case 's':
	case 'v':	goto yy24;
	default:	goto yy14;
	}
yy19:
	yyaccept = 1;
	yych = *(YYMARKER = ++s);
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy20;
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
	case 'z':
		yyt1 = s;
		goto yy30;
	default:	goto yy14;
	}
yy20:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy20;
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
	case 'z':
		yyt1 = s;
		goto yy30;
	default:	goto yy22;
	}
yy22:
	s = YYMARKER;
	if (yyaccept == 0) {
		goto yy17;
	} else {
		goto yy14;
	}
yy23:
	yych = *++s;
yy24:
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy23;
	case 'a':
		yyt3 = s;
		goto yy25;
	case 'n':
		yyt3 = s;
		goto yy26;
	case 'o':
		yyt3 = s;
		goto yy27;
	case 's':
		yyt3 = s;
		goto yy28;
	case 'v':
		yyt3 = s;
		goto yy29;
	default:	goto yy22;
	}
yy25:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy32;
	default:	goto yy22;
	}
yy26:
	yych = *++s;
	switch (yych) {
	case 'u':	goto yy33;
	default:	goto yy22;
	}
yy27:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy34;
	default:	goto yy22;
	}
yy28:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy35;
	default:	goto yy22;
	}
yy29:
	yych = *++s;
	switch (yych) {
	case 'o':	goto yy36;
	default:	goto yy22;
	}
yy30:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt2 = s;
		goto yy37;
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
	case 'z':	goto yy30;
	case ';':
		yyt2 = s;
		goto yy39;
	default:	goto yy22;
	}
yy32:
	yych = *++s;
	switch (yych) {
	case 'y':	goto yy41;
	default:	goto yy22;
	}
yy33:
	yych = *++s;
	switch (yych) {
	case 'm':	goto yy42;
	default:	goto yy22;
	}
yy34:
	yych = *++s;
	switch (yych) {
	case 'j':	goto yy43;
	default:	goto yy22;
	}
yy35:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy44;
	default:	goto yy22;
	}
yy36:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy45;
	default:	goto yy22;
	}
yy37:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy37;
	case ';':	goto yy39;
	default:	goto yy22;
	}
yy39:
	++s;
	a = yyt3;
	b = yyt4;
	c = yyt1;
	d = yyt2;
	state = yycmodule;
	{
            if (a && b) {
                copy_string(buffers[1], a, b);
            } else {
                strcpy(buffers[1], "void");
            }
            callbacks->end_function(context, buffers[0], buffers[1], copy_string(buffers[2], c, d));
            continue;
        }
yy41:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt4 = s;
		goto yy46;
	case '=':
		yyt4 = s;
		goto yy20;
	default:	goto yy22;
	}
yy42:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy48;
	default:	goto yy22;
	}
yy43:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy49;
	default:	goto yy22;
	}
yy44:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy50;
	default:	goto yy22;
	}
yy45:
	yych = *++s;
	switch (yych) {
	case 'd':	goto yy41;
	default:	goto yy22;
	}
yy46:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy46;
	case '=':	goto yy20;
	default:	goto yy22;
	}
yy48:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy51;
	default:	goto yy22;
	}
yy49:
	yych = *++s;
	switch (yych) {
	case 'c':	goto yy52;
	default:	goto yy22;
	}
yy50:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy53;
	default:	goto yy22;
	}
yy51:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy41;
	default:	goto yy22;
	}
yy52:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy41;
	default:	goto yy22;
	}
yy53:
	yych = *++s;
	switch (yych) {
	case 'g':	goto yy41;
	default:	goto yy22;
	}
/* *********************************** */
yyc_function:
	yych = *s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy58;
	case ')':	goto yy61;
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
	case 'z':
		yyt1 = s;
		goto yy63;
	default:	goto yy56;
	}
yy56:
	++s;
yy57:
	{ return false; }
yy58:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy58;
	default:	goto yy60;
	}
yy60:
	{ continue; }
yy61:
	++s;
	state = yycfret;
	goto yyc_fret;
yy63:
	yych = *(YYMARKER = ++s);
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
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
	case ':':
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
	case 'z':	goto yy68;
	default:	goto yy57;
	}
yy64:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy64;
	case ':':	goto yy69;
	default:	goto yy66;
	}
yy66:
	s = YYMARKER;
	goto yy57;
yy67:
	yych = *++s;
yy68:
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt2 = s;
		goto yy64;
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
	case 'z':	goto yy67;
	case ':':
		yyt2 = s;
		goto yy69;
	default:	goto yy66;
	}
yy69:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy69;
	case 'a':
		yyt3 = s;
		goto yy71;
	case 'n':
		yyt3 = s;
		goto yy72;
	case 'o':
		yyt3 = s;
		goto yy73;
	case 's':
		yyt3 = s;
		goto yy74;
	case 'v':
		yyt3 = s;
		goto yy75;
	default:	goto yy66;
	}
yy71:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy76;
	default:	goto yy66;
	}
yy72:
	yych = *++s;
	switch (yych) {
	case 'u':	goto yy77;
	default:	goto yy66;
	}
yy73:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy78;
	default:	goto yy66;
	}
yy74:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy79;
	default:	goto yy66;
	}
yy75:
	yych = *++s;
	switch (yych) {
	case 'o':	goto yy80;
	default:	goto yy66;
	}
yy76:
	yych = *++s;
	switch (yych) {
	case 'y':	goto yy81;
	default:	goto yy66;
	}
yy77:
	yych = *++s;
	switch (yych) {
	case 'm':	goto yy82;
	default:	goto yy66;
	}
yy78:
	yych = *++s;
	switch (yych) {
	case 'j':	goto yy83;
	default:	goto yy66;
	}
yy79:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy84;
	default:	goto yy66;
	}
yy80:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy85;
	default:	goto yy66;
	}
yy81:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt4 = s;
		goto yy86;
	case ')':
	case ',':
		yyt5 = NULL;
		yyt4 = s;
		goto yy88;
	case '.':
		yyt4 = s;
		goto yy90;
	default:	goto yy66;
	}
yy82:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy91;
	default:	goto yy66;
	}
yy83:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy92;
	default:	goto yy66;
	}
yy84:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy93;
	default:	goto yy66;
	}
yy85:
	yych = *++s;
	switch (yych) {
	case 'd':	goto yy81;
	default:	goto yy66;
	}
yy86:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy86;
	case ')':
	case ',':
		yyt5 = NULL;
		goto yy88;
	case '.':	goto yy90;
	default:	goto yy66;
	}
yy88:
	++s;
	a = yyt1;
	b = yyt2;
	c = yyt3;
	d = yyt4;
	f = yyt5;
	s -= 1;
	e = yyt5;
	if (yyt5 != NULL) e -= 3;
	state = yycfcomma;
	{
            callbacks->function_arg(context, copy_string(buffers[1], a, b), copy_string(buffers[2], c, d), e != NULL);
            continue;
        }
yy90:
	yych = *++s;
	switch (yych) {
	case '.':	goto yy94;
	default:	goto yy66;
	}
yy91:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy95;
	default:	goto yy66;
	}
yy92:
	yych = *++s;
	switch (yych) {
	case 'c':	goto yy96;
	default:	goto yy66;
	}
yy93:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy97;
	default:	goto yy66;
	}
yy94:
	yych = *++s;
	switch (yych) {
	case '.':	goto yy98;
	default:	goto yy66;
	}
yy95:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy81;
	default:	goto yy66;
	}
yy96:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy81;
	default:	goto yy66;
	}
yy97:
	yych = *++s;
	switch (yych) {
	case 'g':	goto yy81;
	default:	goto yy66;
	}
yy98:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt5 = s;
		goto yy99;
	case ')':
	case ',':
		yyt5 = s;
		goto yy88;
	default:	goto yy66;
	}
yy99:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy99;
	case ')':
	case ',':	goto yy88;
	default:	goto yy66;
	}
/* *********************************** */
yyc_init:
	yych = *s;
	switch (yych) {
	case 0x00:	goto yy103;
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy107;
	case '#':	goto yy110;
	case 'm':	goto yy111;
	case 'p':	goto yy112;
	default:	goto yy105;
	}
yy103:
	++s;
	{ return true; }
yy105:
	++s;
yy106:
	{ return false; }
yy107:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy107;
	default:	goto yy109;
	}
yy109:
	{ continue; }
yy110:
	yych = *(YYMARKER = ++s);
	if (yych <= 0x00) goto yy106;
	goto yy114;
yy111:
	yych = *(YYMARKER = ++s);
	switch (yych) {
	case 'o':	goto yy118;
	default:	goto yy106;
	}
yy112:
	yych = *(YYMARKER = ++s);
	switch (yych) {
	case 'r':	goto yy119;
	default:	goto yy106;
	}
yy113:
	yych = *++s;
yy114:
	switch (yych) {
	case 0x00:	goto yy115;
	case '\n':	goto yy116;
	default:	goto yy113;
	}
yy115:
	s = YYMARKER;
	goto yy106;
yy116:
	++s;
	{ /* comment */ continue; }
yy118:
	yych = *++s;
	switch (yych) {
	case 'd':	goto yy120;
	default:	goto yy115;
	}
yy119:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy121;
	default:	goto yy115;
	}
yy120:
	yych = *++s;
	switch (yych) {
	case 'u':	goto yy122;
	default:	goto yy115;
	}
yy121:
	yych = *++s;
	switch (yych) {
	case 'f':	goto yy123;
	default:	goto yy115;
	}
yy122:
	yych = *++s;
	switch (yych) {
	case 'l':	goto yy124;
	default:	goto yy115;
	}
yy123:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy125;
	default:	goto yy115;
	}
yy124:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy126;
	default:	goto yy115;
	}
yy125:
	yych = *++s;
	switch (yych) {
	case 'x':	goto yy127;
	default:	goto yy115;
	}
yy126:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy129;
	default:	goto yy115;
	}
yy127:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy127;
	case '"':
		yyt1 = s;
		goto yy131;
	default:	goto yy115;
	}
yy129:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy129;
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
	case 'z':
		yyt1 = s;
		goto yy133;
	default:	goto yy115;
	}
yy131:
	yych = *++s;
	switch (yych) {
	case 0x00:	goto yy115;
	case '"':	goto yy135;
	default:	goto yy131;
	}
yy133:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt2 = s;
		goto yy136;
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
	case 'z':	goto yy133;
	case '{':
		yyt2 = s;
		goto yy138;
	default:	goto yy115;
	}
yy135:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt2 = s;
		goto yy140;
	case ';':
		yyt2 = s;
		goto yy142;
	default:	goto yy115;
	}
yy136:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy136;
	case '{':	goto yy138;
	default:	goto yy115;
	}
yy138:
	++s;
	a = yyt1;
	b = yyt2;
	state = yycmodule;
	{ callbacks->begin_module(context, copy_string(buffers[0], a, b)); continue; }
yy140:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy140;
	case ';':	goto yy142;
	default:	goto yy115;
	}
yy142:
	++s;
	a = yyt1;
	b = yyt2;
	{ callbacks->prefix(context, copy_string(buffers[0], a, b)); continue; }
/* *********************************** */
yyc_module:
	yych = *s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy148;
	case '#':	goto yy151;
	case 'c':	goto yy152;
	case 'f':	goto yy153;
	case 'p':	goto yy154;
	case '}':	goto yy155;
	default:	goto yy146;
	}
yy146:
	++s;
yy147:
	{ return false; }
yy148:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy148;
	default:	goto yy150;
	}
yy150:
	{ continue; }
yy151:
	yych = *(YYMARKER = ++s);
	if (yych <= 0x00) goto yy147;
	goto yy158;
yy152:
	yych = *(YYMARKER = ++s);
	switch (yych) {
	case 'o':	goto yy162;
	default:	goto yy147;
	}
yy153:
	yych = *(YYMARKER = ++s);
	switch (yych) {
	case 'n':	goto yy163;
	default:	goto yy147;
	}
yy154:
	yych = *(YYMARKER = ++s);
	switch (yych) {
	case 'r':	goto yy164;
	default:	goto yy147;
	}
yy155:
	++s;
	state = yycinit;
	{ callbacks->end_module(context); continue; }
yy157:
	yych = *++s;
yy158:
	switch (yych) {
	case 0x00:	goto yy159;
	case '\n':	goto yy160;
	default:	goto yy157;
	}
yy159:
	s = YYMARKER;
	goto yy147;
yy160:
	++s;
	{ /* comment */ continue; }
yy162:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy165;
	default:	goto yy159;
	}
yy163:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy166;
	default:	goto yy159;
	}
yy164:
	yych = *++s;
	switch (yych) {
	case 'o':	goto yy168;
	default:	goto yy159;
	}
yy165:
	yych = *++s;
	switch (yych) {
	case 's':	goto yy169;
	default:	goto yy159;
	}
yy166:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy166;
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
	case 'z':
		yyt1 = s;
		goto yy170;
	default:	goto yy159;
	}
yy168:
	yych = *++s;
	switch (yych) {
	case 'p':	goto yy172;
	default:	goto yy159;
	}
yy169:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy174;
	default:	goto yy159;
	}
yy170:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt2 = s;
		goto yy175;
	case '(':
		yyt2 = s;
		goto yy177;
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
	case 'z':	goto yy170;
	default:	goto yy159;
	}
yy172:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy172;
	case '(':	goto yy180;
	default:	goto yy159;
	}
yy174:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy182;
	default:	goto yy159;
	}
yy175:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy175;
	case '(':	goto yy177;
	default:	goto yy159;
	}
yy177:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy177;
	default:	goto yy179;
	}
yy179:
	a = yyt1;
	b = yyt2;
	state = yycfunction;
	{
            copy_string(buffers[0], a, b);
            callbacks->begin_function(context, buffers[0]);
            continue;
        }
yy180:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy180;
	case 'r':
		yyt1 = s;
		goto yy184;
	case 'w':
		yyt1 = s;
		goto yy185;
	default:	goto yy159;
	}
yy182:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy182;
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
	case 'z':
		yyt1 = s;
		goto yy186;
	default:	goto yy159;
	}
yy184:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy188;
	default:	goto yy159;
	}
yy185:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy189;
	default:	goto yy159;
	}
yy186:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt2 = s;
		goto yy190;
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
	case 'z':	goto yy186;
	case ':':
		yyt2 = s;
		goto yy192;
	default:	goto yy159;
	}
yy188:
	yych = *++s;
	switch (yych) {
	case 'a':	goto yy194;
	default:	goto yy159;
	}
yy189:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy195;
	default:	goto yy159;
	}
yy190:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy190;
	case ':':	goto yy192;
	default:	goto yy159;
	}
yy192:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy192;
	case 'a':
		yyt3 = s;
		goto yy196;
	case 'n':
		yyt3 = s;
		goto yy197;
	case 'o':
		yyt3 = s;
		goto yy198;
	case 's':
		yyt3 = s;
		goto yy199;
	case 'v':
		yyt3 = s;
		goto yy200;
	default:	goto yy159;
	}
yy194:
	yych = *++s;
	switch (yych) {
	case 'd':	goto yy201;
	default:	goto yy159;
	}
yy195:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy202;
	default:	goto yy159;
	}
yy196:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy203;
	default:	goto yy159;
	}
yy197:
	yych = *++s;
	switch (yych) {
	case 'u':	goto yy204;
	default:	goto yy159;
	}
yy198:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy205;
	default:	goto yy159;
	}
yy199:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy206;
	default:	goto yy159;
	}
yy200:
	yych = *++s;
	switch (yych) {
	case 'o':	goto yy207;
	default:	goto yy159;
	}
yy201:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt2 = s;
		goto yy208;
	case ')':
		yyt2 = s;
		goto yy210;
	default:	goto yy159;
	}
yy202:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy201;
	default:	goto yy159;
	}
yy203:
	yych = *++s;
	switch (yych) {
	case 'y':	goto yy212;
	default:	goto yy159;
	}
yy204:
	yych = *++s;
	switch (yych) {
	case 'm':	goto yy213;
	default:	goto yy159;
	}
yy205:
	yych = *++s;
	switch (yych) {
	case 'j':	goto yy214;
	default:	goto yy159;
	}
yy206:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy215;
	default:	goto yy159;
	}
yy207:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy216;
	default:	goto yy159;
	}
yy208:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy208;
	case ')':	goto yy210;
	default:	goto yy159;
	}
yy210:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy210;
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
	case 'z':
		yyt3 = s;
		goto yy217;
	default:	goto yy159;
	}
yy212:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt4 = s;
		goto yy219;
	case '=':
		yyt4 = s;
		goto yy221;
	default:	goto yy159;
	}
yy213:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy223;
	default:	goto yy159;
	}
yy214:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy224;
	default:	goto yy159;
	}
yy215:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy225;
	default:	goto yy159;
	}
yy216:
	yych = *++s;
	switch (yych) {
	case 'd':	goto yy212;
	default:	goto yy159;
	}
yy217:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt4 = s;
		goto yy226;
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
	case 'z':	goto yy217;
	case ':':
		yyt4 = s;
		goto yy228;
	default:	goto yy159;
	}
yy219:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy219;
	case '=':	goto yy221;
	default:	goto yy159;
	}
yy221:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy221;
	case '"':
		yyt5 = s;
		goto yy230;
	case '-':
		yyt5 = s;
		goto yy232;
	case '0':
		yyt5 = s;
		goto yy233;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		yyt5 = s;
		goto yy234;
	default:	goto yy159;
	}
yy223:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy236;
	default:	goto yy159;
	}
yy224:
	yych = *++s;
	switch (yych) {
	case 'c':	goto yy237;
	default:	goto yy159;
	}
yy225:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy238;
	default:	goto yy159;
	}
yy226:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy226;
	case ':':	goto yy228;
	default:	goto yy159;
	}
yy228:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy228;
	case 'a':
		yyt5 = s;
		goto yy239;
	case 'n':
		yyt5 = s;
		goto yy240;
	case 'o':
		yyt5 = s;
		goto yy241;
	case 's':
		yyt5 = s;
		goto yy242;
	case 'v':
		yyt5 = s;
		goto yy243;
	default:	goto yy159;
	}
yy230:
	yych = *++s;
	switch (yych) {
	case 0x00:	goto yy159;
	case '"':	goto yy244;
	default:	goto yy230;
	}
yy232:
	yych = *++s;
	switch (yych) {
	case '0':	goto yy233;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy234;
	default:	goto yy159;
	}
yy233:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt6 = s;
		goto yy245;
	case ';':
		yyt6 = s;
		goto yy247;
	case 'x':	goto yy249;
	default:	goto yy159;
	}
yy234:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt6 = s;
		goto yy245;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy234;
	case ';':
		yyt6 = s;
		goto yy247;
	default:	goto yy159;
	}
yy236:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy212;
	default:	goto yy159;
	}
yy237:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy212;
	default:	goto yy159;
	}
yy238:
	yych = *++s;
	switch (yych) {
	case 'g':	goto yy212;
	default:	goto yy159;
	}
yy239:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy250;
	default:	goto yy159;
	}
yy240:
	yych = *++s;
	switch (yych) {
	case 'u':	goto yy251;
	default:	goto yy159;
	}
yy241:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy252;
	default:	goto yy159;
	}
yy242:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy253;
	default:	goto yy159;
	}
yy243:
	yych = *++s;
	switch (yych) {
	case 'o':	goto yy254;
	default:	goto yy159;
	}
yy244:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt6 = s;
		goto yy245;
	case ';':
		yyt6 = s;
		goto yy247;
	default:	goto yy159;
	}
yy245:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy245;
	case ';':	goto yy247;
	default:	goto yy159;
	}
yy247:
	++s;
	a = yyt1;
	b = yyt2;
	c = yyt3;
	d = yyt4;
	e = yyt5;
	f = yyt6;
	{
            copy_string(buffers[2], e, f);
            if (buffers[2][0] == '"') {
                callbacks->constant_string(context, copy_string(buffers[0], a, b), copy_string(buffers[1], c, d), buffers[2]);
            } else if (isdigit(buffers[2][0])) {
                if (buffers[2][1] == 'x') {
                    callbacks->constant_number(context, copy_string(buffers[0], a, b), copy_string(buffers[1], c, d), strtol(buffers[2]+2, NULL, 16));
                } else {
                    callbacks->constant_number(context, copy_string(buffers[0], a, b), copy_string(buffers[1], c, d), strtol(buffers[2], NULL, 10));
                }
            }
            continue;
        }
yy249:
	yych = *++s;
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
	case 'f':	goto yy255;
	default:	goto yy159;
	}
yy250:
	yych = *++s;
	switch (yych) {
	case 'y':	goto yy257;
	default:	goto yy159;
	}
yy251:
	yych = *++s;
	switch (yych) {
	case 'm':	goto yy258;
	default:	goto yy159;
	}
yy252:
	yych = *++s;
	switch (yych) {
	case 'j':	goto yy259;
	default:	goto yy159;
	}
yy253:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy260;
	default:	goto yy159;
	}
yy254:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy261;
	default:	goto yy159;
	}
yy255:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt6 = s;
		goto yy245;
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
	case 'f':	goto yy255;
	case ';':
		yyt6 = s;
		goto yy247;
	default:	goto yy159;
	}
yy257:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt6 = s;
		goto yy262;
	case '=':
		yyt6 = s;
		goto yy264;
	default:	goto yy159;
	}
yy258:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy266;
	default:	goto yy159;
	}
yy259:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy267;
	default:	goto yy159;
	}
yy260:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy268;
	default:	goto yy159;
	}
yy261:
	yych = *++s;
	switch (yych) {
	case 'd':	goto yy257;
	default:	goto yy159;
	}
yy262:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy262;
	case '=':	goto yy264;
	default:	goto yy159;
	}
yy264:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy264;
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
	case 'z':
		yyt7 = s;
		goto yy269;
	default:	goto yy159;
	}
yy266:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy271;
	default:	goto yy159;
	}
yy267:
	yych = *++s;
	switch (yych) {
	case 'c':	goto yy272;
	default:	goto yy159;
	}
yy268:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy273;
	default:	goto yy159;
	}
yy269:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt8 = s;
		goto yy274;
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
	case 'z':	goto yy269;
	case ';':
		yyt8 = s;
		goto yy276;
	default:	goto yy159;
	}
yy271:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy257;
	default:	goto yy159;
	}
yy272:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy257;
	default:	goto yy159;
	}
yy273:
	yych = *++s;
	switch (yych) {
	case 'g':	goto yy257;
	default:	goto yy159;
	}
yy274:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy274;
	case ';':	goto yy276;
	default:	goto yy159;
	}
yy276:
	++s;
	a = yyt1;
	b = yyt2;
	c = yyt3;
	d = yyt4;
	e = yyt5;
	f = yyt6;
	g = yyt7;
	h = yyt8;
	{
            callbacks->property(
                context,
                copy_string(buffers[0], c, d),
                copy_string(buffers[1], e, f),
                copy_string(buffers[2], a, b),
                copy_string(buffers[3], g, h)
            );
            continue;
        }
}

    }
}