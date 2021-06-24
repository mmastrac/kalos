#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <kalos_run.h>
#include "avr_dispatch.inc"

const uint8_t __flash SCRIPT[] = {
#include "avr_script.inc"
};

int main(void) {
    kalos_script script = {
        .buffer = SCRIPT
    };
    kalos_run_state* run_state = kalos_init(&script, NULL, NULL);
    avr_trigger_main(run_state);
    kalos_run_free(run_state);
}
