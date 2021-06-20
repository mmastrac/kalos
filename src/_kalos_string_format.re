#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "_kalos_string_format.h"

// Comparison equivalent to 'unsigned == 0'
#pragma warning 136 9

bool kalos_parse_string_format(const char* s, kalos_string_format* string_format) {
    const char* YYMARKER;
    const char *fill, *align, *sign, *alt_fmt, *zero_pad, *min_width, *precision, *type;

    /*!stags:re2c format = "const char *@@{tag}; "; */

    /*!re2c
        re2c:api:style = free-form;
        re2c:define:YYCTYPE = char;
        re2c:define:YYCURSOR = s;
        re2c:yyfill:enable = 0;
        re2c:tags:expression = "@@{tag}";

        int = "0" | [1-9][0-9]*;

        * { return false; }

        (
            (@fill [^\x00])?
            @align ("^" | [<>=])
        )?
        (@sign [+\- ])?
        (@alt_fmt "#")?
        (@zero_pad "0")?
        (@min_width int)?
        ("." @precision int)?
        (@type [cdoxX])? "\x00"
        {
            memset(string_format, 0, sizeof(*string_format));
            if (zero_pad) {
                string_format->fill = '0';
                string_format->align = '>';
            }
            if (fill) {
                string_format->fill = *fill;
            }
            if (align) {
                string_format->align = *align;
            }
            if (sign) {
                string_format->sign = *sign;
            }
            if (alt_fmt) {
                string_format->alt_fmt = *alt_fmt;
            }
            if (min_width) {
				string_format->min_width = 0;
                while (isdigit(*min_width)) {
                    string_format->min_width *= 10;
                    string_format->min_width += (*min_width) - '0';
                    min_width++;
                }
            }
            if (precision) {
				string_format->precision = 0;
                while (isdigit(*precision)) {
                    string_format->precision *= 10;
                    string_format->precision += (*precision) - '0';
                    precision++;
                }
            }
            if (type) {
                string_format->type = *type;
            }
            return true;
        }

    */
}
