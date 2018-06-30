#include "naivejson.h"
#include <assert.h> /* assert() */
#include <stdio.h>
#include <stdlib.h> /* NULL, malloc(), realloc(), free(), strtod() */
#include <string.h> /* memcpy() */
#include <errno.h>  /* errno, ERANGE */
#include <math.h>  /* HUGE_VAL */


#ifndef NAIVE_PARSE_STACK_INIT_SIZE
#define NAIVE_PARSE_STACK_INIT_SIZE 256
#endif

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch) do {*(char*) naive_context_push(c, sizeof(char)) = (ch);} while(0)

#define EXPECT(c, ch) do { assert(*c->json ==(ch)); c->json++; } while(0)

void naive_init(naive_value* v) {
    v->type = NAIVE_NULL;
}

static void* naive_context_push(naive_context* c, size_t size) {
    void* ret;
    assert(size > 0);
    if (c->top + size >= c->size) {
        if (c->size == 0)
            c->size = NAIVE_PARSE_STACK_INIT_SIZE;
        while (c->top + size >= c->size)
            c->size += c->size >> 1;  /* c->size * 1.5 */
        c->stack = (char*)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

static void* naive_context_pop(naive_context*c, size_t size) {
    assert(c->top >= size);
    c->top -= size;
    return c->stack + (c->top);
}


static void naive_parse_whitespace(naive_context* c) {
    const char *p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json =p;
}


static int naive_parse_literal(naive_context* c, naive_value* v, const char* literal, naive_type type) {
    size_t  i;
    EXPECT(c, literal[0]);
    for(i = 0; literal[i + 1]; i++)
        if (c->json[i] != literal[i+1])
            return NAIVE_PARSE_INVALID_VALUE;
    c->json += i;
    v->type = type;
    return NAIVE_PARSE_OK;
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
        case 't':   return naive_parse_literal(c, v, "true", NAIVE_TRUE);
        case 'f':   return naive_parse_literal(c, v, "false", NAIVE_FALSE);
        case 'n':   return naive_parse_literal(c, v, "null", NAIVE_NULL);
        case '"':   return naive_parse_string(c, v);
        case '\0':  return NAIVE_PARSE_EXPECT_VALUE;
        case '[' :  return naive_parse_array(c, v);
        case '{' :  return naive_parse_object(c, v);
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
    const char* p = c->json;
    if (*p == '-')  p++;

    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return NAIVE_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }

    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return NAIVE_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    errno = 0;
    v->u.n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
        return NAIVE_PARSE_NUMBER_TOO_BIG;
    v->type = NAIVE_NUMBER;
    c->json = p;
    return NAIVE_PARSE_OK;
}

static int naive_parse_string(naive_context* c, naive_value* v) {
    size_t head = c->top, len;
    const char* p;
    EXPECT(c, '\"');
    p = c->json;
    for(;;) {
        char ch = *p;
        p += 1;
        switch(ch) {
            case '\"':
                len = c->top - head;
                naive_set_string(v, (const char*)naive_context_pop(c, len),len);
                c->json = p;
                return NAIVE_PARSE_OK;
            case '\\':
                switch (*p++){
                    case '\"': PUTC(c, '\"'); break;
                    case '\\': PUTC(c, '\\'); break;
                    case 'b':  PUTC(c, '\b'); break;
                    case 'f':  PUTC(c, '\f'); break;
                    case 'n':  PUTC(c, '\n'); break;
                    case 'r':  PUTC(c, '\r'); break;
                    case 't':  PUTC(c, '\t'); break;
                }
            case '\0':
                c->top = head;
                return NAIVE_PARSE_MISS_QUOTATION_MARK;
            default:
                PUTC(c, ch);
        }
    }
}




naive_type naive_parse(naive_value* v, const char* json) {
    naive_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    naive_init(v);
    naive_parse_whitespace(&c);
    if ((ret = naive_parse_value(&c, v)) == NAIVE_PARSE_OK) {
        naive_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = NAIVE_NULL;
            ret = NAIVE_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    free(c.stack);
    return ret;
}

naive_type naive_get_type(const naive_value* v) {
    assert(v != NULL);
    return v->type;
}

double naive_get_number(const naive_value *v) {
    assert(v != NULL && v->type == NAIVE_NUMBER);
    return v->u.n;
}

void naive_set_string(naive_value* v, const char* s, size_t len){
    assert(v != NULL && (s != NULL || len == 0));
    naive_free(v);
    v->u.s.s = (char* )malloc(len+1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = NAIVE_STRING;
}

const char* naive_get_string(const naive_value* v) {
    assert(v != NULL && v->type == NAIVE_STRING);
    return v->u.s.s;
}

size_t naive_get_string_length(const naive_value* v) {
    assert(v != NULL && v->type == NAIVE_STRING);
    return v->u.s.len;
}

size_t naive_get_array_size(const naive_value* v) {
    assert(v != NULL && v->type == NAIVE_ARRAY);
    return v->u.a.size;
}

naive_value* naive_get_array_element(const naive_value* v, size_t index) {
    assert(v != NULL && v->type == NAIVE_ARRAY);
    assert(index < v->u.a.size);
    return &v->u.a.e[index];
}

void naive_free(naive_value* v) {
    size_t i;
    switch(v->type) {
        case NAIVE_STRING:
            free(v->u.s.s);
            break;
        case NAIVE_ARRAY:
            for (i = 0; i < v->u.a.size; i++)
                naive_free(&v->u.a.e[i]);
            free(v->u.a.e);
            break;
        default:
            break;
    }
    v->type = NAIVE_NULL;
}

static int naive_parse_array(naive_context* c, naive_value* v) {
    size_t size = 0;
    int ret;
    EXPECT(c, '[');
    if (*c->json == ']') {
        c->json++;
        v->type = NAIVE_ARRAY;
        v->u.a.size = 0;
        v->u.a.e = NULL;
        return NAIVE_PARSE_OK;
    }
    for (;;) {
        naive_value e;
        naive_init(&e);
        if ((ret = naive_parse_value(c, &e)) != NAIVE_PARSE_OK)
            return ret;
        memcpy(naive_context_push(c, sizeof(naive_value)), &e, sizeof(naive_value));
        size++;
        if (*c->json == ',')
            c->json++;
        else if (*c->json == ']') {
            c->json++;
            v->type = NAIVE_ARRAY;
            v->u.a.size = size;
            size *= sizeof(naive_value);
            memcpy(v->u.a.e = (naive_value*)malloc(size), naive_context_pop(c, size), size);
            return NAIVE_PARSE_OK;
        }
        else
            return NAIVE_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
    }
}

int naive_get_boolean(const naive_value* v) {
    assert(v != NULL && (v->type == NAIVE_TRUE || v->type == NAIVE_FALSE));
    return v->type == NAIVE_TRUE;
}

void naive_set_boolean(naive_value* v, int b) {
    naive_free(v);
    v->type = b ? NAIVE_TRUE : NAIVE_FALSE;
}

void naive_set_number(naive_value* v, double n) {
    naive_free(v);
    v->u.n = n;
    v->type = NAIVE_NUMBER;
}

static int naive_parse_object(naive_context* c, naive_value* v) {
    size_t size;
    naive_member m;
    int ret;
    EXPECT(c, '{');
    naive_parse_whitespace(c);
    if (*c->json == '}') {
        c->json++;
        v->type = NAIVE_OBJECT;
        v->u.o.m = 0;
        v->u.o.size = 0;
        return NAIVE_PARSE_OK;
    }
    m.k = NULL;
    size = 0;
    for (;;){
        naive_init(&m.v);
        if ((ret == naive_parse_value(c, &m.v) != NAIVE_PARSE_OK))
            break;
        memcpy(naive_context_push(c, sizeof(naive_member)), &m, sizeof(naive_member));
        size++;
        m.k = NULL;
    }
    return ret;
}

size_t naive_get_object_key_length(const naive_value* v, size_t index) {
    return v->u.o.m[index].klen;
}

naive_value* lept_get_object_value(const naive_value* v, size_t index) {
    return &v->u.o.m[index].v;
}