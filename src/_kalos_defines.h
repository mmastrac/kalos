#pragma once

#include <stdlib.h>
#include <string.h>

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

// https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms
#define KM__CHECK_N(x, n, ...) n
#define KM__CHECK(...) KM__CHECK_N(__VA_ARGS__, 0,)
#define KM__PROBE(x) x, 1,
#define KM__IS_PAREN(x) KM__CHECK(KM__IS_PAREN_PROBE x)
#define KM__IS_PAREN_PROBE(...) KM__PROBE(~)
#define KM__PRIMITIVE_COMPARE(x, y) KM__IS_PAREN ( KM__COMPARE_ ## x ( KM__COMPARE_ ## y) (()) )
#define KM__COMPARE_(x)
#define KM__COMPARE_0(x)

// Magic
#define KALOS_EVAL__(...) __VA_ARGS__
#define KALOS_CONCAT_P__(a, ...) a ## __VA_ARGS__
#define KALOS_CONCAT__(a, ...) KALOS_CONCAT_P__(a, __VA_ARGS__)
#define KALOS_COUNTER__(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77, _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96, _97, _98, _99, N, ...) N
#define KALOS_COUNT__(...) KALOS_COUNTER__(__VA_ARGS__, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define KALOS_FOREACH__(X, ...) KALOS_FOREACH_N__(X, __VA_ARGS__,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,)
#define KALOS_FOREACH_N__(X, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77, _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96, _97, _98, _99, ...) \
    X(0,_0) X(1,_1) X(2,_2) X(3,_3) X(4,_4) X(5,_5) X(6,_6) X(7,_7) X(8,_8) X(9,_9) X(10,_10) X(11,_11) X(12,_12) X(13,_13) X(14,_14) X(15,_15) X(16,_16) X(17,_17) X(18,_18) X(19,_19) X(20,_20) X(21,_21) X(22,_22) X(23,_23) X(24,_24) X(25,_25) X(26,_26) X(27,_27) X(28,_28) X(29,_29) X(30,_30) X(31,_31) X(32,_32) X(33,_33) X(34,_34) X(35,_35) X(36,_36) X(37,_37) X(38,_38) X(39,_39) X(40,_40) X(41,_41) X(42,_42) X(43,_43) X(44,_44) X(45,_45) X(46,_46) X(47,_47) X(48,_48) X(49,_49) X(50,_50) X(51,_51) X(52,_52) X(53,_53) X(54,_54) X(55,_55) X(56,_56) X(57,_57) X(58,_58) X(59,_59) X(60,_60) X(61,_61) X(62,_62) X(63,_63) X(64,_64) X(65,_65) X(66,_66) X(67,_67) X(68,_68) X(69,_69) X(70,_70) X(71,_71) X(72,_72) X(73,_73) X(74,_74) X(75,_75) X(76,_76) X(77,_77) X(78,_78) X(79,_79) X(80,_80) X(81,_81) X(82,_82) X(83,_83) X(84,_84) X(85,_85) X(86,_86) X(87,_87) X(88,_88) X(89,_89) X(90,_90) X(91,_91) X(92,_92) X(93,_93) X(94,_94) X(95,_95) X(96,_96) X(97,_97) X(98,_98) X(99,_99)
#define KALOS_FOREACH2__(X, ...) KALOS_FOREACH2_N__(X, __VA_ARGS__,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,)
#define KALOS_FOREACH2_N__(X, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77, _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96, _97, _98, _99, ...) \
    X(0,_0) X(1,_1) X(2,_2) X(3,_3) X(4,_4) X(5,_5) X(6,_6) X(7,_7) X(8,_8) X(9,_9) X(10,_10) X(11,_11) X(12,_12) X(13,_13) X(14,_14) X(15,_15) X(16,_16) X(17,_17) X(18,_18) X(19,_19) X(20,_20) X(21,_21) X(22,_22) X(23,_23) X(24,_24) X(25,_25) X(26,_26) X(27,_27) X(28,_28) X(29,_29) X(30,_30) X(31,_31) X(32,_32) X(33,_33) X(34,_34) X(35,_35) X(36,_36) X(37,_37) X(38,_38) X(39,_39) X(40,_40) X(41,_41) X(42,_42) X(43,_43) X(44,_44) X(45,_45) X(46,_46) X(47,_47) X(48,_48) X(49,_49) X(50,_50) X(51,_51) X(52,_52) X(53,_53) X(54,_54) X(55,_55) X(56,_56) X(57,_57) X(58,_58) X(59,_59) X(60,_60) X(61,_61) X(62,_62) X(63,_63) X(64,_64) X(65,_65) X(66,_66) X(67,_67) X(68,_68) X(69,_69) X(70,_70) X(71,_71) X(72,_72) X(73,_73) X(74,_74) X(75,_75) X(76,_76) X(77,_77) X(78,_78) X(79,_79) X(80,_80) X(81,_81) X(82,_82) X(83,_83) X(84,_84) X(85,_85) X(86,_86) X(87,_87) X(88,_88) X(89,_89) X(90,_90) X(91,_91) X(92,_92) X(93,_93) X(94,_94) X(95,_95) X(96,_96) X(97,_97) X(98,_98) X(99,_99)

#define PTR_BYTE_OFFSET(ptr, offset) (void*)(((uint8_t*)ptr)+(ptrdiff_t)offset)
#define PTR_BYTE_OFFSET_NEG(ptr, offset) (void*)(((uint8_t*)ptr)-(ptrdiff_t)offset)
#define PTR_BYTE_SUBTRACT(ptr1, ptr2) (((uint8_t*)ptr1) - ((uint8_t*)ptr2))
