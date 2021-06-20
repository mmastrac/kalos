#include <memory.h>
#include <ctype.h>
#include "_kalos_idl_parse.h"
#include "_kalos_module.h"

// Comparison equivalent to 'unsigned == 0'
#pragma warning 136 9

enum yyc_state {
    yycinit,
    yycmodule,
    yycfunction,
    yycfcomma,
    yycfret,
    yychandle,
    yychcomma,
    yycobject,
};

const char* copy_string(char* buffer, const char* a, const char* b) {
    memcpy(buffer, a, b-a);
    buffer[b-a] = 0;
    return (const char*)buffer;
}

bool kalos_idl_parse_callback(const char* s, void* context, kalos_idl_callbacks* callbacks) {
    const char* start = s;
    const char* YYMARKER;
    const char *ws, *a, *b, *c, *d, *e, *f, *g, *h;
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
        type = "number" | "string" | "bool" | "void" | "any" | "object";

        <init,module,object,function,fcomma,fret,handle,hcomma> * { callbacks->error(context, start, s - start); return false; }
        <init,module,object,function,fcomma,fret,handle,hcomma> ws { continue; }
        <init> end { return true; }
        <init,module,object> "#" [^\n\x00]* "\n" { /* comment */ continue; }
        <init> "prefix" ws? @a string @b ws? ";" { if (!callbacks->prefix(context, copy_string(buffers[0], a, b))) return false; continue; }
        <init> "dispatch" ws? "name" ws? ";" { callbacks->dispatch_name(context); }
        <init> "module" ws @a word @b ws? "{" => module { callbacks->begin_module(context, copy_string(buffers[0], a, b)); continue; }
        <module> "object" ws? @a word @b ws? "{" => object { callbacks->begin_object(context, copy_string(buffers[0], a, b)); continue; } 
        <object> "}" => module { callbacks->end_object(context); continue; }
        <module,object> "prop" ws? "(" ws? @a mode @b ws? ")" ws? @c word @d ws? ":" ws? @e type @f ws? "=" ws? @g word @h ws? ";" {
            callbacks->property(
                context,
                copy_string(buffers[0], c, d), // name
                copy_string(buffers[1], e, f), // type
                copy_string(buffers[2], a, b), // mode
                copy_string(buffers[3], g, h),
                NULL
            );
            continue;
        }
        <module,object> "prop" ws? "(" ws? "read" ws? "," ws? "write" ws? ")" ws? @a word @b ws? ":" ws? @c type @d ws? "=" ws? @e word @f ws? "," ws? @g word @h @ws? ";" {
            callbacks->property(
                context,
                copy_string(buffers[0], a, b),  // name
                copy_string(buffers[1], c, d), // type
                "read,write", // mode
                copy_string(buffers[2], e, f), // symbol1
                copy_string(buffers[3], g, h) // symbol2
            );
            continue;
        }
        <module> "const" ws @a word @b ws? ":" ws? @c type @d ws? "=" ws? @e const @f ws? ";" {
            copy_string(buffers[2], e, f);
            if (buffers[2][0] == '"') {
                if (!callbacks->constant_string(context, copy_string(buffers[0], a, b), copy_string(buffers[1], c, d), buffers[2])) {
                    return false;
                }
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
        <fret> ( ":" ws? @a type @b )? ws? "=" ws? "C" ws? @c string @d ws? ";" => module {
            if (a && b) {
                copy_string(buffers[1], a, b);
            } else {
                strcpy(buffers[1], "void");
            }
            callbacks->end_function_c(context, buffers[0], buffers[1], copy_string(buffers[2], c, d));
            continue;
        }
        <module> "handle" ws @a word @b ws? "(" ws? => handle {
            copy_string(buffers[0], a, b);
            callbacks->begin_handle(context, buffers[0]);
            continue;
        }
        <module> "handle" ws @a word @b ";" {
            copy_string(buffers[0], a, b);
            callbacks->begin_handle(context, buffers[0]);
            callbacks->end_handle(context);
            continue;
        }
        <handle> @a word @b ws? ":" ws? @c type @d ws? (@e "..." @f)? ws? / ","|")" => hcomma {
            callbacks->handle_arg(context, copy_string(buffers[1], a, b), copy_string(buffers[2], c, d), e != NULL);
            continue;
        }
        <hcomma> "," :=> handle
        <hcomma,handle> ")" ws? ";" => module {
            callbacks->end_handle(context);
        }
        <module> "}" => init { callbacks->end_module(context); continue; }
    */
    }
}
