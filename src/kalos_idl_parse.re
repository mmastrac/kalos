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

    /*!stags:re2c format = "const char *@@{tag}; "; */

    for (;;) {
    /*!re2c
        re2c:api:style = free-form;
        re2c:define:YYCTYPE = char;
        re2c:define:YYCURSOR = s;
        re2c:yyfill:enable = 0;
        re2c:define:YYGETCONDITION = "state";
        re2c:define:YYSETCONDITION = "state = @@;";
        re2c:tags:expression = "@@{tag}";

        end = "\x00";
        int = "-"? ("0" | [1-9][0-9]*);
        hex = "-"? "0x" [0-9a-fA-F]+;
        word = [a-zA-Z_][a-zA-Z0-9_]*;
        string = '"' [^"\x00]* '"';
        const = int | hex | string;
        ws = (" " | "\t" | "\r" | "\n")+;
        mode = "read" | "write";
        type = "number" | "string" | "void" | "any" | "object";

        <init,module,function,fcomma,fret> * { return false; }
        <init,module,function,fcomma,fret> ws { continue; }
        <init> end { return true; }
        <init,module> "#" [^\n\x00]* "\n" { /* comment */ continue; }
        <init> "prefix" ws? @a string @b ws? ";" { callbacks->prefix(context, copy_string(buffers[0], a, b)); continue; }
        <init> "module" ws @a word @b ws? "{" => module { callbacks->begin_module(context, copy_string(buffers[0], a, b)); continue; }
        <module> "prop" ws? "(" ws? @a mode @b ws? ")" ws? @c word @d ws? ":" ws? @e type @f ws? "=" ws? @g word @h ws? ";" {
            callbacks->property(
                context,
                copy_string(buffers[0], c, d),
                copy_string(buffers[1], e, f),
                copy_string(buffers[2], a, b),
                copy_string(buffers[3], g, h)
            );
            continue;
        }
        <module> "const" ws @a word @b ws? ":" ws? @c type @d ws? "=" ws? @e const @f ws? ";" {
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
        <module> "fn" ws @a word @b ws? "(" ws? => function {
            copy_string(buffers[0], a, b);
            callbacks->begin_function(context, buffers[0]);
            continue;
        }
        <function> @a word @b ws? ":" ws? @c type @d ws? (@e "..." @f)? ws? / ","|")" => fcomma {
            callbacks->function_arg(context, copy_string(buffers[1], a, b), copy_string(buffers[2], c, d), e != NULL);
            continue;
        }
        <function> ")" :=> fret
        <fcomma> "," :=> function
        <fcomma> ")" :=> fret
        <fret> ( ":" ws? @a type @b )? ws? "=" ws? @c word @d ws? ";" => module {
            if (a && b) {
                copy_string(buffers[1], a, b);
            } else {
                strcpy(buffers[1], "void");
            }
            callbacks->end_function(context, buffers[0], buffers[1], copy_string(buffers[2], c, d));
            continue;
        }
        <module> "}" => init { callbacks->end_module(context); continue; }
    */
    }
}
