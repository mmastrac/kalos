#pragma once

/** @file
 * The Kalos virtual machine for running compiled scripts.
 */

#include "kalos.h"
#include "_kalos_value.h"
#include "_kalos_module.h"
#include "_kalos_script.h"

/**
 * Initialize the Kalos virtual machine.
 */
kalos_run_state* kalos_init(const_kalos_script script, kalos_dispatch* dispatch, kalos_state* state);
/**
 * Trigger a kalos export. Usually not called by anything but the generated IDL code.
 */
void kalos_trigger(kalos_run_state* state, kalos_export_address handler_address);
/**
 * Free the Kalos virtual machine internal state.
 */
void kalos_run_free(kalos_run_state* state);
/**
 * Trigger an internal error, halting the virtual machine and reporting to the host environment.
 */
void kalos_internal_error(kalos_state* state);
/**
 * Trigger a type error, halting the virtual machine and reporting to the host environment.
 */
void kalos_type_error(kalos_state* state);
/**
 * Trigger a value error, halting the virtual machine and reporting to the host environment.
 */
void kalos_value_error(kalos_state* state);
