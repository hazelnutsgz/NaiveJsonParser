#ifndef NAIVEJSON_H__
#define NAIVEJSON_H__

#define EXPECT(c, ch) do { assert(*c->json ==(ch)); c->json++; } while(0)


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
    naive_type type;
} naive_value;

typedef struct {
    const char *json;
} naive_context;


enum {
    NAIVE_PARSE_OK = 0, //NORMAL
    NAIVE_PARSE_EXPECT_VALUE, //If the json only contains vacuum
    NAIVE_PARSE_INVALID_VALUE,
    NAIVE_PARSE_ROOT_NOT_SINGULAR //
};


//The parser should not modify the original input string, so we use const identifier
//The return value is the state of parse.
naive_type naive_parse(naive_value *v, const char *json);

naive_type naive_get_type(const naive_value *v);


#endif