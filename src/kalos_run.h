#pragma once

#include <stddef.h>
#include <stdint.h>
#include "kalos.h"
#include "kalos_value.h"
#include "kalos_module.h"

kalos_run_state* kalos_init(kalos_script* script, kalos_dispatch* dispatch, kalos_state* state);
void kalos_trigger(kalos_run_state* state, kalos_export_address handle_address);
void kalos_run_free(kalos_run_state* state);
void kalos_internal_error(kalos_state* state);
void kalos_type_error(kalos_state* state);
void kalos_value_error(kalos_state* state);
