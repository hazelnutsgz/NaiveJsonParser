#include "naivejson.h"

#include <assert.h>
#include <stdio.h>

static void naive_parse_whitespace(naive_context* c) {
    const char *p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json =p;
}

static naive_type naive_parse_null(naive_context* c, naive_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return NAIVE_PARSE_INVALID_VALUE;

    c->json += 3;
    v->type = NAIVE_NULL;

    return NAIVE_PARSE_OK;
}

static naive_type naive_parse_value(naive_context* c, naive_value* v) {
    switch (*(c->json)) {
        case 'n':   return naive_parse_null(c, v);
        case '\0':  return NAIVE_PARSE_EXPECT_VALUE;
        default:    return NAIVE_PARSE_INVALID_VALUE;
    }
}

naive_type naive_parse(naive_value* v, const char* json) {
    naive_context c;
    assert(v != NULL);
    c.json = json;
    v->type = NAIVE_NULL;
    naive_parse_whitespace(&c);
    return naive_parse_value(&c, v);
}

naive_type naive_get_type(const naive_value* v) {
    assert(v != NULL);
    return v->type;
}