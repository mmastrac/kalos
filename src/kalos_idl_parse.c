/* Generated by re2c 2.1.1 on Sun May 30 13:02:43 2021 */
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
	case 'a':
		yyt3 = s;
		goto yy23;
	case 'n':
		yyt3 = s;
		goto yy24;
	case 'o':
		yyt3 = s;
		goto yy25;
	case 's':
		yyt3 = s;
		goto yy26;
	case 'v':
		yyt3 = s;
		goto yy27;
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
		goto yy28;
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
		goto yy28;
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
	switch (yych) {
	case 'n':	goto yy30;
	default:	goto yy22;
	}
yy24:
	yych = *++s;
	switch (yych) {
	case 'u':	goto yy31;
	default:	goto yy22;
	}
yy25:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy32;
	default:	goto yy22;
	}
yy26:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy33;
	default:	goto yy22;
	}
yy27:
	yych = *++s;
	switch (yych) {
	case 'o':	goto yy34;
	default:	goto yy22;
	}
yy28:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt2 = s;
		goto yy35;
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
	case 'z':	goto yy28;
	case ';':
		yyt2 = s;
		goto yy37;
	default:	goto yy22;
	}
yy30:
	yych = *++s;
	switch (yych) {
	case 'y':	goto yy39;
	default:	goto yy22;
	}
yy31:
	yych = *++s;
	switch (yych) {
	case 'm':	goto yy40;
	default:	goto yy22;
	}
yy32:
	yych = *++s;
	switch (yych) {
	case 'j':	goto yy41;
	default:	goto yy22;
	}
yy33:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy42;
	default:	goto yy22;
	}
yy34:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy43;
	default:	goto yy22;
	}
yy35:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy35;
	case ';':	goto yy37;
	default:	goto yy22;
	}
yy37:
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
yy39:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt4 = s;
		goto yy44;
	case '=':
		yyt4 = s;
		goto yy20;
	default:	goto yy22;
	}
yy40:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy46;
	default:	goto yy22;
	}
yy41:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy47;
	default:	goto yy22;
	}
yy42:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy48;
	default:	goto yy22;
	}
yy43:
	yych = *++s;
	switch (yych) {
	case 'd':	goto yy39;
	default:	goto yy22;
	}
yy44:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy44;
	case '=':	goto yy20;
	default:	goto yy22;
	}
yy46:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy49;
	default:	goto yy22;
	}
yy47:
	yych = *++s;
	switch (yych) {
	case 'c':	goto yy50;
	default:	goto yy22;
	}
yy48:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy51;
	default:	goto yy22;
	}
yy49:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy39;
	default:	goto yy22;
	}
yy50:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy39;
	default:	goto yy22;
	}
yy51:
	yych = *++s;
	switch (yych) {
	case 'g':	goto yy39;
	default:	goto yy22;
	}
/* *********************************** */
yyc_function:
	yych = *s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy56;
	case ')':	goto yy59;
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
		goto yy61;
	default:	goto yy54;
	}
yy54:
	++s;
yy55:
	{ return false; }
yy56:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy56;
	default:	goto yy58;
	}
yy58:
	{ continue; }
yy59:
	++s;
	state = yycfret;
	goto yyc_fret;
yy61:
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
	case 'z':	goto yy66;
	default:	goto yy55;
	}
yy62:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy62;
	case ':':	goto yy67;
	default:	goto yy64;
	}
yy64:
	s = YYMARKER;
	goto yy55;
yy65:
	yych = *++s;
yy66:
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt2 = s;
		goto yy62;
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
	case 'z':	goto yy65;
	case ':':
		yyt2 = s;
		goto yy67;
	default:	goto yy64;
	}
yy67:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy67;
	case 'a':
		yyt3 = s;
		goto yy69;
	case 'n':
		yyt3 = s;
		goto yy70;
	case 'o':
		yyt3 = s;
		goto yy71;
	case 's':
		yyt3 = s;
		goto yy72;
	case 'v':
		yyt3 = s;
		goto yy73;
	default:	goto yy64;
	}
yy69:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy74;
	default:	goto yy64;
	}
yy70:
	yych = *++s;
	switch (yych) {
	case 'u':	goto yy75;
	default:	goto yy64;
	}
yy71:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy76;
	default:	goto yy64;
	}
yy72:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy77;
	default:	goto yy64;
	}
yy73:
	yych = *++s;
	switch (yych) {
	case 'o':	goto yy78;
	default:	goto yy64;
	}
yy74:
	yych = *++s;
	switch (yych) {
	case 'y':	goto yy79;
	default:	goto yy64;
	}
yy75:
	yych = *++s;
	switch (yych) {
	case 'm':	goto yy80;
	default:	goto yy64;
	}
yy76:
	yych = *++s;
	switch (yych) {
	case 'j':	goto yy81;
	default:	goto yy64;
	}
yy77:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy82;
	default:	goto yy64;
	}
yy78:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy83;
	default:	goto yy64;
	}
yy79:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt4 = s;
		goto yy84;
	case ')':
	case ',':
		yyt4 = s;
		goto yy86;
	default:	goto yy64;
	}
yy80:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy88;
	default:	goto yy64;
	}
yy81:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy89;
	default:	goto yy64;
	}
yy82:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy90;
	default:	goto yy64;
	}
yy83:
	yych = *++s;
	switch (yych) {
	case 'd':	goto yy79;
	default:	goto yy64;
	}
yy84:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy84;
	case ',':	goto yy86;
	default:	goto yy64;
	}
yy86:
	++s;
	a = yyt1;
	b = yyt2;
	c = yyt3;
	s = yyt4;
	d = yyt4;
	state = yycfcomma;
	{
            callbacks->function_arg(context, copy_string(buffers[1], a, b), copy_string(buffers[2], c, d));
            continue;
        }
yy88:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy91;
	default:	goto yy64;
	}
yy89:
	yych = *++s;
	switch (yych) {
	case 'c':	goto yy92;
	default:	goto yy64;
	}
yy90:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy93;
	default:	goto yy64;
	}
yy91:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy79;
	default:	goto yy64;
	}
yy92:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy79;
	default:	goto yy64;
	}
yy93:
	yych = *++s;
	switch (yych) {
	case 'g':	goto yy79;
	default:	goto yy64;
	}
/* *********************************** */
yyc_init:
	yych = *s;
	switch (yych) {
	case 0x00:	goto yy96;
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy100;
	case '#':	goto yy103;
	case 'm':	goto yy104;
	default:	goto yy98;
	}
yy96:
	++s;
	{ return true; }
yy98:
	++s;
yy99:
	{ return false; }
yy100:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy100;
	default:	goto yy102;
	}
yy102:
	{ continue; }
yy103:
	yych = *(YYMARKER = ++s);
	if (yych <= 0x00) goto yy99;
	goto yy106;
yy104:
	yych = *(YYMARKER = ++s);
	switch (yych) {
	case 'o':	goto yy110;
	default:	goto yy99;
	}
yy105:
	yych = *++s;
yy106:
	switch (yych) {
	case 0x00:	goto yy107;
	case '\n':	goto yy108;
	default:	goto yy105;
	}
yy107:
	s = YYMARKER;
	goto yy99;
yy108:
	++s;
	{ /* comment */ continue; }
yy110:
	yych = *++s;
	switch (yych) {
	case 'd':	goto yy111;
	default:	goto yy107;
	}
yy111:
	yych = *++s;
	switch (yych) {
	case 'u':	goto yy112;
	default:	goto yy107;
	}
yy112:
	yych = *++s;
	switch (yych) {
	case 'l':	goto yy113;
	default:	goto yy107;
	}
yy113:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy114;
	default:	goto yy107;
	}
yy114:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy115;
	default:	goto yy107;
	}
yy115:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy115;
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
		goto yy117;
	default:	goto yy107;
	}
yy117:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt2 = s;
		goto yy119;
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
	case 'z':	goto yy117;
	case '{':
		yyt2 = s;
		goto yy121;
	default:	goto yy107;
	}
yy119:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy119;
	case '{':	goto yy121;
	default:	goto yy107;
	}
yy121:
	++s;
	a = yyt1;
	b = yyt2;
	state = yycmodule;
	{ callbacks->begin_module(context, copy_string(buffers[0], a, b)); continue; }
/* *********************************** */
yyc_module:
	yych = *s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy127;
	case '#':	goto yy130;
	case 'c':	goto yy131;
	case 'f':	goto yy132;
	case 'p':	goto yy133;
	case '}':	goto yy134;
	default:	goto yy125;
	}
yy125:
	++s;
yy126:
	{ return false; }
yy127:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy127;
	default:	goto yy129;
	}
yy129:
	{ continue; }
yy130:
	yych = *(YYMARKER = ++s);
	if (yych <= 0x00) goto yy126;
	goto yy137;
yy131:
	yych = *(YYMARKER = ++s);
	switch (yych) {
	case 'o':	goto yy141;
	default:	goto yy126;
	}
yy132:
	yych = *(YYMARKER = ++s);
	switch (yych) {
	case 'n':	goto yy142;
	default:	goto yy126;
	}
yy133:
	yych = *(YYMARKER = ++s);
	switch (yych) {
	case 'r':	goto yy143;
	default:	goto yy126;
	}
yy134:
	++s;
	state = yycinit;
	goto yyc_init;
yy136:
	yych = *++s;
yy137:
	switch (yych) {
	case 0x00:	goto yy138;
	case '\n':	goto yy139;
	default:	goto yy136;
	}
yy138:
	s = YYMARKER;
	goto yy126;
yy139:
	++s;
	{ /* comment */ continue; }
yy141:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy144;
	default:	goto yy138;
	}
yy142:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy145;
	default:	goto yy138;
	}
yy143:
	yych = *++s;
	switch (yych) {
	case 'o':	goto yy147;
	default:	goto yy138;
	}
yy144:
	yych = *++s;
	switch (yych) {
	case 's':	goto yy148;
	default:	goto yy138;
	}
yy145:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy145;
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
		goto yy149;
	default:	goto yy138;
	}
yy147:
	yych = *++s;
	switch (yych) {
	case 'p':	goto yy151;
	default:	goto yy138;
	}
yy148:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy153;
	default:	goto yy138;
	}
yy149:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt2 = s;
		goto yy154;
	case '(':
		yyt2 = s;
		goto yy156;
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
	case 'z':	goto yy149;
	default:	goto yy138;
	}
yy151:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy151;
	case '(':	goto yy159;
	default:	goto yy138;
	}
yy153:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy161;
	default:	goto yy138;
	}
yy154:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy154;
	case '(':	goto yy156;
	default:	goto yy138;
	}
yy156:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy156;
	default:	goto yy158;
	}
yy158:
	a = yyt1;
	b = yyt2;
	state = yycfunction;
	{
            copy_string(buffers[0], a, b);
            callbacks->begin_function(context, buffers[0]);
            continue;
        }
yy159:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy159;
	case 'r':
		yyt1 = s;
		goto yy163;
	case 'w':
		yyt1 = s;
		goto yy164;
	default:	goto yy138;
	}
yy161:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy161;
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
		goto yy165;
	default:	goto yy138;
	}
yy163:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy167;
	default:	goto yy138;
	}
yy164:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy168;
	default:	goto yy138;
	}
yy165:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt2 = s;
		goto yy169;
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
	case 'z':	goto yy165;
	case ':':
		yyt2 = s;
		goto yy171;
	default:	goto yy138;
	}
yy167:
	yych = *++s;
	switch (yych) {
	case 'a':	goto yy173;
	default:	goto yy138;
	}
yy168:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy174;
	default:	goto yy138;
	}
yy169:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy169;
	case ':':	goto yy171;
	default:	goto yy138;
	}
yy171:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy171;
	case 'a':
		yyt3 = s;
		goto yy175;
	case 'n':
		yyt3 = s;
		goto yy176;
	case 'o':
		yyt3 = s;
		goto yy177;
	case 's':
		yyt3 = s;
		goto yy178;
	case 'v':
		yyt3 = s;
		goto yy179;
	default:	goto yy138;
	}
yy173:
	yych = *++s;
	switch (yych) {
	case 'd':	goto yy180;
	default:	goto yy138;
	}
yy174:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy181;
	default:	goto yy138;
	}
yy175:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy182;
	default:	goto yy138;
	}
yy176:
	yych = *++s;
	switch (yych) {
	case 'u':	goto yy183;
	default:	goto yy138;
	}
yy177:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy184;
	default:	goto yy138;
	}
yy178:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy185;
	default:	goto yy138;
	}
yy179:
	yych = *++s;
	switch (yych) {
	case 'o':	goto yy186;
	default:	goto yy138;
	}
yy180:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt2 = s;
		goto yy187;
	case ')':
		yyt2 = s;
		goto yy189;
	default:	goto yy138;
	}
yy181:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy180;
	default:	goto yy138;
	}
yy182:
	yych = *++s;
	switch (yych) {
	case 'y':	goto yy191;
	default:	goto yy138;
	}
yy183:
	yych = *++s;
	switch (yych) {
	case 'm':	goto yy192;
	default:	goto yy138;
	}
yy184:
	yych = *++s;
	switch (yych) {
	case 'j':	goto yy193;
	default:	goto yy138;
	}
yy185:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy194;
	default:	goto yy138;
	}
yy186:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy195;
	default:	goto yy138;
	}
yy187:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy187;
	case ')':	goto yy189;
	default:	goto yy138;
	}
yy189:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy189;
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
		goto yy196;
	default:	goto yy138;
	}
yy191:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt4 = s;
		goto yy198;
	case '=':
		yyt4 = s;
		goto yy200;
	default:	goto yy138;
	}
yy192:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy202;
	default:	goto yy138;
	}
yy193:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy203;
	default:	goto yy138;
	}
yy194:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy204;
	default:	goto yy138;
	}
yy195:
	yych = *++s;
	switch (yych) {
	case 'd':	goto yy191;
	default:	goto yy138;
	}
yy196:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt4 = s;
		goto yy205;
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
	case 'z':	goto yy196;
	case ':':
		yyt4 = s;
		goto yy207;
	default:	goto yy138;
	}
yy198:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy198;
	case '=':	goto yy200;
	default:	goto yy138;
	}
yy200:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy200;
	case '"':
		yyt5 = s;
		goto yy209;
	case '0':
		yyt5 = s;
		goto yy211;
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
		goto yy212;
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
		yyt5 = s;
		goto yy214;
	default:	goto yy138;
	}
yy202:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy216;
	default:	goto yy138;
	}
yy203:
	yych = *++s;
	switch (yych) {
	case 'c':	goto yy217;
	default:	goto yy138;
	}
yy204:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy218;
	default:	goto yy138;
	}
yy205:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy205;
	case ':':	goto yy207;
	default:	goto yy138;
	}
yy207:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy207;
	case 'a':
		yyt5 = s;
		goto yy219;
	case 'n':
		yyt5 = s;
		goto yy220;
	case 'o':
		yyt5 = s;
		goto yy221;
	case 's':
		yyt5 = s;
		goto yy222;
	case 'v':
		yyt5 = s;
		goto yy223;
	default:	goto yy138;
	}
yy209:
	yych = *++s;
	switch (yych) {
	case 0x00:	goto yy138;
	case '"':	goto yy211;
	default:	goto yy209;
	}
yy211:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt6 = s;
		goto yy224;
	case ';':
		yyt6 = s;
		goto yy226;
	default:	goto yy138;
	}
yy212:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt6 = s;
		goto yy224;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy212;
	case ';':
		yyt6 = s;
		goto yy226;
	default:	goto yy138;
	}
yy214:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt6 = s;
		goto yy224;
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
	case 'z':	goto yy214;
	case ';':
		yyt6 = s;
		goto yy226;
	default:	goto yy138;
	}
yy216:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy191;
	default:	goto yy138;
	}
yy217:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy191;
	default:	goto yy138;
	}
yy218:
	yych = *++s;
	switch (yych) {
	case 'g':	goto yy191;
	default:	goto yy138;
	}
yy219:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy228;
	default:	goto yy138;
	}
yy220:
	yych = *++s;
	switch (yych) {
	case 'u':	goto yy229;
	default:	goto yy138;
	}
yy221:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy230;
	default:	goto yy138;
	}
yy222:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy231;
	default:	goto yy138;
	}
yy223:
	yych = *++s;
	switch (yych) {
	case 'o':	goto yy232;
	default:	goto yy138;
	}
yy224:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy224;
	case ';':	goto yy226;
	default:	goto yy138;
	}
yy226:
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
                callbacks->constant_number(context, copy_string(buffers[0], a, b), copy_string(buffers[1], c, d), strtol(buffers[2], NULL, 10));
            } else {
                callbacks->constant_symbol(context, copy_string(buffers[0], a, b), copy_string(buffers[1], c, d), buffers[2]);
            }
            continue;
        }
yy228:
	yych = *++s;
	switch (yych) {
	case 'y':	goto yy233;
	default:	goto yy138;
	}
yy229:
	yych = *++s;
	switch (yych) {
	case 'm':	goto yy234;
	default:	goto yy138;
	}
yy230:
	yych = *++s;
	switch (yych) {
	case 'j':	goto yy235;
	default:	goto yy138;
	}
yy231:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy236;
	default:	goto yy138;
	}
yy232:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy237;
	default:	goto yy138;
	}
yy233:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt6 = s;
		goto yy238;
	case '=':
		yyt6 = s;
		goto yy240;
	default:	goto yy138;
	}
yy234:
	yych = *++s;
	switch (yych) {
	case 'b':	goto yy242;
	default:	goto yy138;
	}
yy235:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy243;
	default:	goto yy138;
	}
yy236:
	yych = *++s;
	switch (yych) {
	case 'i':	goto yy244;
	default:	goto yy138;
	}
yy237:
	yych = *++s;
	switch (yych) {
	case 'd':	goto yy233;
	default:	goto yy138;
	}
yy238:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy238;
	case '=':	goto yy240;
	default:	goto yy138;
	}
yy240:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy240;
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
		goto yy245;
	default:	goto yy138;
	}
yy242:
	yych = *++s;
	switch (yych) {
	case 'e':	goto yy247;
	default:	goto yy138;
	}
yy243:
	yych = *++s;
	switch (yych) {
	case 'c':	goto yy248;
	default:	goto yy138;
	}
yy244:
	yych = *++s;
	switch (yych) {
	case 'n':	goto yy249;
	default:	goto yy138;
	}
yy245:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		yyt8 = s;
		goto yy250;
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
	case 'z':	goto yy245;
	case ';':
		yyt8 = s;
		goto yy252;
	default:	goto yy138;
	}
yy247:
	yych = *++s;
	switch (yych) {
	case 'r':	goto yy233;
	default:	goto yy138;
	}
yy248:
	yych = *++s;
	switch (yych) {
	case 't':	goto yy233;
	default:	goto yy138;
	}
yy249:
	yych = *++s;
	switch (yych) {
	case 'g':	goto yy233;
	default:	goto yy138;
	}
yy250:
	yych = *++s;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy250;
	case ';':	goto yy252;
	default:	goto yy138;
	}
yy252:
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
