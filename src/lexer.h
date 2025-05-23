#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_IDENT,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_TYPE, 
    TOKEN_KEYWORD,
    TOKEN_OPERATOR,
    TOKEN_SYMBOL,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *text;
} Token;

Token *tokenize(const char *source, int *token_count);

void free_tokens(Token *tokens, int token_count);

#endif