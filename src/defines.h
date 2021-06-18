#pragma once

#ifdef __WATCOMC__
#pragma off (unreferenced)
#pragma on (reuse_duplicate_strings)
#else
// Allow modern IDEs to type-check our code
#ifndef __huge
#define __huge
#endif
#ifndef __far
#define __far
#endif
// These macros suck, but watcom provides them
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#define DEBUG 1
#ifdef IS_TEST
#include <assert.h>
#define ASSERT(...) assert(__VA_ARGS__)
void test_log(const char* msg, ...);
#define LOG(...) {test_log(__FILE__, __LINE__, __VA_ARGS__);}
#else
// #define ASSERT(x) { if (!(x)) { printf("%s", "ASSERTION FAILED: " #x "\n"); } else { printf("%s", "OK " #x "\n"); } }
void log_printf(const char* msg, ...);
#ifdef DEBUG
#define ASSERT(x) { if (!(x)) { printf("%s", "ASSERTION FAILED: " #x "\n"); } }
#define LOG(...) log_printf(__VA_ARGS__)
#else
#define ASSERT(x) if (sizeof(x)) {}
#define LOG(...)
#endif

#endif
