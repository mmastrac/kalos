#pragma once

#include <stdbool.h>
#include "kalos_parse.h"

void kalos_dump(kalos_script* script, char* buffer);

typedef bool (*kalos_walk_fn)(void* context, kalos_script* script, kalos_export_address handle_address, uint16_t pc);
void kalos_walk(kalos_script* script, void* context, kalos_walk_fn fn);
uint16_t kalos_find_section(kalos_script* script, kalos_export_address handle_address);
