#include "naivejson.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
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
        case 't':   return naive_parse_true(c, v);
        case 'f':   return naive_parse_false(c, v);
        case 'n':   return naive_parse_null(c, v);
        case '\0':  return NAIVE_PARSE_EXPECT_VALUE;
        default:    return naive_parse_number(c, v);
    }
}

static int naive_parse_true(naive_context* c, naive_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return NAIVE_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = NAIVE_TRUE;
    return NAIVE_PARSE_OK;
}

static int naive_parse_false(naive_context* c, naive_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return NAIVE_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = NAIVE_FALSE;
    return NAIVE_PARSE_OK;
}

static int naive_parse_number(naive_context* c, naive_value* v) {
    char* end;
    v->n = strtod(c->json, &end);
    if (c->json == end)
        return NAIVE_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = NAIVE_NUMBER;
    return NAIVE_PARSE_OK;
}






naive_type naive_parse(naive_value* v, const char* json) {
    naive_context c;
    assert(v != NULL);
    c.json = json;
    v->type = NAIVE_NULL;
    naive_parse_whitespace(&c);

    int ret;
    if ((ret = naive_parse_value(&c, v)) == NAIVE_PARSE_OK) {
        naive_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = NAIVE_NULL;
            ret = NAIVE_PARSE_ROOT_NOT_SINGULAR;
        }
    }

    return ret;
}

naive_type naive_get_type(const naive_value* v) {
    assert(v != NULL);
    return v->type;
}

double naive_get_number(const naive_value *v) {
    assert(v != NULL && v->type == NAIVE_NUMBER);
    return v->n;
}