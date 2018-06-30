//
// Created by 佘国榛 on 2018/6/27.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "naivejson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

#define TEST_NUMBER(expect, json)\
    do {\
        naive_value v;\
        EXPECT_EQ_INT(NAIVE_PARSE_OK, naive_parse(&v, json));\
        EXPECT_EQ_INT(NAIVE_NUMBER, naive_get_type(&v));\
        EXPECT_EQ_DOUBLE(expect, naive_get_number(&v));\
    } while(0)

#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == alength && memcmp(expect, actual, alength) == 0, expect, actual, "%s")

#define TEST_ERROR(error, json)\
    do {\
        naive_value v;\
        naive_init(&v);\
        v.type = NAIVE_FALSE;\
        EXPECT_EQ_INT(error, naive_parse(&v, json));\
        EXPECT_EQ_INT(NAIVE_NULL, naive_get_type(&v));\
        naive_free(&v);\
    } while(0)

#define TEST_STRING(expect, json)\
    do {\
        naive_value v;\
        naive_init(&v);\
        EXPECT_EQ_INT(NAIVE_PARSE_OK, naive_parse(&v, json));\
        EXPECT_EQ_INT(NAIVE_STRING, naive_get_type(&v));\
        EXPECT_EQ_STRING(expect, naive_get_string(&v), naive_get_string_length(&v));\
        naive_free(&v);\
    } while(0)


static void test_parse_null() {
    naive_value v;
    v.type = NAIVE_TRUE;
    EXPECT_EQ_INT(NAIVE_PARSE_OK, naive_parse(&v, "null"));
    EXPECT_EQ_INT(NAIVE_NULL, naive_get_type(&v));

}



static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
//    TEST_NUMBER(1E10, "1E10");
//    TEST_NUMBER(1e10, "1e10");
//    TEST_NUMBER(1E+10, "1E+10");
//    TEST_NUMBER(1E-10, "1E-10");
//    TEST_NUMBER(-1E10, "-1E10");
//    TEST_NUMBER(-1e10, "-1e10");
//    TEST_NUMBER(-1E+10, "-1E+10");
//    TEST_NUMBER(-1E-10, "-1E-10");
//    TEST_NUMBER(1.234E+10, "1.234E+10");
//    TEST_NUMBER(1.234E-10, "1.234E-10");
//    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
}

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\nWorld\"");
}


static void test_parse_true() {
    naive_value v;
    v.type = NAIVE_FALSE;
    EXPECT_EQ_INT(NAIVE_PARSE_OK, naive_parse(&v, "null"));
}

static void test_access_string() {
    int length;
    naive_value v;
    naive_init(&v);
    naive_set_string(&v, "", 0);
    length = naive_get_string_length(&v);
    EXPECT_EQ_STRING("", naive_get_string(&v), length);
    naive_set_string(&v, "Hello", 5);
    length = naive_get_string_length(&v);
    EXPECT_EQ_STRING("Hello", naive_get_string(&v), length);
    naive_free(&v);
}

static void test_parse_expect_value() {
    TEST_ERROR(NAIVE_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(NAIVE_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_ERROR(NAIVE_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(NAIVE_PARSE_INVALID_VALUE, "?");
    TEST_ERROR(NAIVE_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(NAIVE_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(NAIVE_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(NAIVE_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(NAIVE_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(NAIVE_PARSE_INVALID_VALUE, "nan");
}

static void test_parse_root_not_singular() {
    TEST_ERROR(NAIVE_PARSE_ROOT_NOT_SINGULAR, "null x");
    TEST_ERROR(NAIVE_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
    TEST_ERROR(NAIVE_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(NAIVE_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_number_too_big() {
    TEST_ERROR(NAIVE_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(NAIVE_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_missing_quotation_mark() {
    TEST_ERROR(NAIVE_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(NAIVE_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
    TEST_ERROR(NAIVE_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(NAIVE_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(NAIVE_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(NAIVE_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    TEST_ERROR(NAIVE_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(NAIVE_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_number();
    test_parse_string();

    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
//    test_parse_invalid_string_escape();
//    test_parse_invalid_string_char();
//
    test_access_string();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}