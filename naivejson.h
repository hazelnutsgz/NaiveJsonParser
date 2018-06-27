#ifndef NAIVEJSON_H__
#define NAIVEJSON_H__

typedef enum {
    NAIVE_NULL,
    NAIVE_FALSE,
    NAIVE_TRUE,
    NAIVE_NUMBER,
    NAIVE_STRING,
    NAIVE_ARRAY,
    NAIVE_OBJECT
} naive_type;

typedef struct{
    naive_type type;
} naive_value;

enum {
    NAIVE_PARSE_OK = 0, //NORMAL
    NAIVE_PARSE_EXPECT_VALUE, //If the json only contains vacuum
    NAIVE_PARSE_INVALID_VALUE,
    NAIVE_PARSE_ROOT_NOT_SINGULAR //
};



//The parser should not modify the original input string, so we use const identifier
//The return value is the state of parse.
int naive_parse(naive_value* v, const char* json);



#endif