#pragma once

#include <stdint.h>

#ifdef __WATCOMC__
#define kalos_far __far
#else
#define kalos_far 
#endif

typedef int16_t kalos_int;
typedef const char* kalos_string;

typedef void* kalos_state;
