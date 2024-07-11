#ifndef __C_TEST_ASSERT_INCLUDED_H__
#define __C_TEST_ASSERT_INCLUDED_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __COLOR_DEFAULT "\x1B[0m"
#define __COLOR_SUCCESS "\x1B[32m" // GREEN
#define __COLOR_FAIL "\x1B[31m" // RED

#define __ASSERT_SUCCESS "SUCCESS"
#define __ASSERT_FAIL "FAIL"

#define __TEST_CNTR \
    int __tcntr = 0
#define __SUCCESS_CNTR \
    int __scntr = 0
#define __FAIL_CNTR \
    int __fcntr = 0

#define __INC_TEST_CNTR \
    __tcntr++
#define __INC_SUCCESS_CNTR \
    __scntr++
#define __INC_FAIL_CNTR \
    __fcntr++

#define __CURR_TEST_CNTR_VAL \
    __tcntr
#define __CURR_SUCCESS_CNTR_VAL \
    __scntr
#define __CURR_FAIL_CNTR_VAL \
    __fcntr

#define __PRINT_MSG(msg, stream, color, status) \
    fprintf(stream, color "%s" __COLOR_DEFAULT ": %s:%d assertion [" msg "]\n", status, __FILE__, __LINE__);

#define __ASSERT(expr)                                                     \
    do {                                                                   \
        __INC_TEST_CNTR;                                                   \
        if (!(expr)) {                                                     \
            __PRINT_MSG(#expr, stdout, __COLOR_FAIL, __ASSERT_FAIL);       \
            __INC_FAIL_CNTR;                                               \
        } else {                                                           \
            __PRINT_MSG(#expr, stdout, __COLOR_SUCCESS, __ASSERT_SUCCESS); \
            __INC_SUCCESS_CNTR;                                            \
        }                                                                  \
    } while (0)

#define ASSERT_TEST_INIT() \
    __TEST_CNTR;           \
    __SUCCESS_CNTR;        \
    __FAIL_CNTR

#define ASSERT_TEST_SUMMARY()                                                                                                               \
    printf(                                                                                                                                 \
        "Total tests: %d\n" __COLOR_SUCCESS "Succeded " __COLOR_DEFAULT "tests: %d\n" __COLOR_FAIL "Failed " __COLOR_DEFAULT "tests: %d\n", \
        (__CURR_TEST_CNTR_VAL), (__CURR_SUCCESS_CNTR_VAL), (__CURR_FAIL_CNTR_VAL))

#define ASSERT_NOT_NULL(expr) \
    __ASSERT((expr) != NULL)

#define ASSERT_NULL(expr) \
    __ASSERT((expr) == NULL)

#define ASSERT_EXPR(expr) \
    __ASSERT(expr)

#define ASSERT_STR_EQ(s1, s2) \
    __ASSERT(strcmp(s1, s2) == 0)

#define ASSERT_STR_S1(s1, s2) \
    __ASSERT(strcmp(s1, s2) == -1)

#define ASSERT_STR_S2(s1, s2) \
    __ASSERT(strcmp(s1, s2) == 1)

#define ASSERT_STR_LEN(s, len) \
    __ASSERT(strlen(s) == len)

#define ASSERT_NUM_VAL(n, val) \
    __ASSERT(n == val)

#define ASSERT_CHAR_VAL(c, val) \
    __ASSERT(c == val)

#define ASSERT_IS_CHAR(val) \
    __ASSERT(val == sizeof(char))

#define ASSERT_IS_SHORT(val) \
    __ASSERT(val == sizeof(short))

#define ASSERT_IS_INT(val) \
    __ASSERT(val == sizeof(int))

#define ASSERT_IS_LONG(val) \
    __ASSERT(val == sizeof(long))

#define ASSERT_IS_UCHAR(val) \
    __ASSERT((val == sizeof(char)) && (val >= 0))

#define ASSERT_IS_USHORT(val) \
    __ASSERT((val == sizeof(short)) && (val >= 0))

#define ASSERT_IS_UINT(val) \
    __ASSERT((val == sizeof(int)) && (val >= 0))

#define ASSERT_IS_ULONG(val) \
    __ASSERT((val == sizeof(long)) && (val >= 0))

#endif // __C_TEST_ASSERT_INCLUDED_H__
