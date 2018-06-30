#ifndef NAIVEJSON_H__
#define NAIVEJSON_H__
#include <stddef.h> /* size_t */

typedef enum {
    NAIVE_NULL,
    NAIVE_FALSE,
    NAIVE_TRUE,
    NAIVE_NUMBER,
    NAIVE_STRING,
    NAIVE_ARRAY,
    NAIVE_OBJECT
} naive_type;

typedef struct {
    char* s;
    size_t len;
    union {
      struct {
          char* s; /* string */
          size_t len;  /* number */
      }s;
      struct {
          naive_type* e;
          size_t size;
      }a;
      double n;
    }u;
    naive_type type;
} naive_value;




typedef struct {
    const char *json;
    char* stack;
    size_t size, top;
} naive_context;


enum {
    NAIVE_PARSE_OK = 0, //NORMAL
    NAIVE_PARSE_EXPECT_VALUE, //If the json only contains vacuum
    NAIVE_PARSE_INVALID_VALUE,
    NAIVE_PARSE_ROOT_NOT_SINGULAR,
    NAIVE_PARSE_NUMBER_TOO_BIG,
    NAIVE_PARSE_MISS_QUOTATION_MARK,
    NAIVE_PARSE_INVALID_STRING_ESCAPE,
    NAIVE_PARSE_INVALID_STRING_CHAR,
    NAIVE_PARSE_MISS_COMMA_OR_SQUARE_BRACKET
};


//The parser should not modify the original input string, so we use const identifier
//The return value is the state of parse.
naive_type naive_parse(naive_value *v, const char *json);

naive_type naive_get_type(const naive_value *v);

double naive_get_number(const naive_value* v);
void naive_set_number(naive_value* v, double n);

const char* naive_get_string(const naive_value* v);
void naive_set_string(naive_value* v, const char* s, size_t len);
size_t naive_get_string_length(const naive_value* v);
void naive_init(naive_value* v);

static void naive_parse_whitespace(naive_context* c);
static naive_type naive_parse_null(naive_context* c, naive_value* v);
static naive_type naive_parse_value(naive_context* c, naive_value* v);
static int naive_parse_true(naive_context* c, naive_value* v);
static int naive_parse_false(naive_context* c, naive_value* v);
static int naive_parse_number(naive_context* c, naive_value* v);
static int naive_parse_string(naive_context* c, naive_value* v);
void naive_free(naive_value* v);


int naive_get_boolean(const naive_value* v);
void naive_set_boolean(naive_value* v, int b);
size_t naive_get_array_size(const naive_value* v);
static int naive_parse_array(naive_context* c, naive_value* v);

#endif