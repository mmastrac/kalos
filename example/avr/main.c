#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <kalos_run.h>
#include "avr_dispatch.inc"

const uint8_t __flash SCRIPT[] = {
#include "avr_script.inc"
};

kalos_state local = {
    .alloc = malloc,
    .realloc = realloc,
    .free = free,
};

int main(void) {
    kalos_dispatch avr_dispatch = {
        .modules = dispatch,
    };
    kalos_run_state* run_state = kalos_init((const_kalos_script)SCRIPT, &avr_dispatch, &local);
    avr_trigger_main(run_state);
    kalos_run_free(run_state);
}
