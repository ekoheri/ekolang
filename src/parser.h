#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef struct {
    Token *tokens;
    int pos;
    int token_count;
} Parser;

typedef enum {
    TYPE_INT,
    TYPE_STRING,
    TYPE_DOUBLE,
    TYPE_UNKNOWN
} VarType;

typedef struct {
    char *name;
    VarType type;
    union {
        int int_val;
        char *str_val;
        double double_val;
    } value;
} Variable;

void parser_init(Parser *p, Token *tokens, int token_count);
void parse_statement(Parser *p);
int peek_token_is(Parser *p, TokenType type, const char *value);
void skip_expression(Parser *p);
void parse_all(Parser *p);

#endif