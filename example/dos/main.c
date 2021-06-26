#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <kalos_run.h>
#define LOG(...)

static uint8_t SCRIPT[] = {
#include "dos_script.inc"
};

void dos_print(kalos_state* state, kalos_string* s) {
    printf("%s", kalos_string_c(state, *s));
}

void dos_println(kalos_state* state, kalos_string* s) {
    printf("%s\n", kalos_string_c(state, *s));
}

#include "dos_dispatch.inc"

static const int verbose = 0;

void log_printf(const char* fmt, ...) {
    if (verbose) {
        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
        fputc('\n', stdout);
    }
}

kalos_state dos_state = {
    .alloc = malloc,
    .realloc = realloc,
    .free = free,
};

int main(void) {
    kalos_dispatch dos_dispatch = {0};
    dos_dispatch.modules = dispatch;
    kalos_run_state* state = kalos_init(SCRIPT, &dos_dispatch, &dos_state);
    dos_trigger_main(state);
    kalos_run_free(state);
}
