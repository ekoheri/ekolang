#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"

#define MAX_TOKEN_LEN 64
#define MAX_TOKENS 1024
#define MAX_FILE_SIZE 8192

Token *tokens = NULL;
int token_count = 0;

const char *keywords[] = {
    "tampil", "jika", "lain", "selama", "fungsi", "benar", "salah"
};

const char *types[] = { "string", "int", "double" };

int line_number = 1;

int is_keyword(const char *word) {
    for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
        if (strcmp(word, keywords[i]) == 0) return 1;
    }
    return 0;
}

int is_type(const char *word) {
    for (int i = 0; i < sizeof(types)/sizeof(types[0]); i++) {
        if (strcmp(word, types[i]) == 0) return 1;
    }
    return 0;
}

void add_token(TokenType type, const char *text) {
    if (token_count >= MAX_TOKENS) return;

    tokens[token_count].type = type;
    tokens[token_count].text = malloc(strlen(text) + 1);
    if (!tokens[token_count].text) {
        fprintf(stderr, "Memory allocation failed for token text\n");
        exit(1);
    }
    strcpy(tokens[token_count].text, text);
    token_count++;
}

void skip_whitespace(const char **src) {
    //while (isspace(**src)) (*src)++;
    while (isspace(**src)) {
        if (**src == '\n') line_number++;
        (*src)++;
    }
}

void read_identifier(const char **src) {
    char buffer[MAX_TOKEN_LEN] = {0};
    int len = 0;
    while (isalnum(**src) || **src == '_') {
        buffer[len++] = *(*src)++;
    }
    buffer[len] = '\0';
    if (is_type(buffer)) add_token(TOKEN_TYPE, buffer);
    else if (is_keyword(buffer)) add_token(TOKEN_KEYWORD, buffer);
    else add_token(TOKEN_IDENT, buffer);
}

void read_number(const char **src) {
    char buffer[MAX_TOKEN_LEN] = {0};
    int len = 0;
    while (isdigit(**src)) {
        buffer[len++] = *(*src)++;
    }
    buffer[len] = '\0';
    add_token(TOKEN_NUMBER, buffer);
}

void read_string(const char **src) {
    char buffer[MAX_TOKEN_LEN] = {0};
    int len = 0;
    (*src)++;  // lewati tanda kutip pembuka

    while (**src && **src != '"') {
        if (**src == '\\') {
            // Tangani escape karakter seperti \" atau \n
            (*src)++;
            if (**src == 'n') buffer[len++] = '\n';
            else if (**src == 't') buffer[len++] = '\t';
            else if (**src == '"') buffer[len++] = '"';
            else if (**src == '\\') buffer[len++] = '\\';
            else buffer[len++] = *(*src);  // karakter setelah '\'
            (*src)++;
        } else if (**src == '\n') {
            buffer[len++] = *(*src)++;
            line_number++;  // hanya update nomor baris, tapi tetap lanjut
        } else {
            buffer[len++] = *(*src)++;
        }

        if (len >= MAX_TOKEN_LEN - 1) break;
    }

    if (**src == '"') (*src)++;  // Lewati tanda kutip penutup jika ada
    buffer[len] = '\0';
    add_token(TOKEN_STRING, buffer);
}

//Mengenali pembacaan operator 1 kali +,= dll dan juga 2 kali &&, ||, dll
void read_operator(const char **src) {
    char buffer[3] = {0};
    buffer[0] = *(*src);

    // Tangani operator dua karakter lebih dulu
    if ((buffer[0] == '&' && *(*src + 1) == '&') ||
        (buffer[0] == '|' && *(*src + 1) == '|') ||
        (buffer[0] == '=' && *(*src + 1) == '=') ||
        (buffer[0] == '!' && *(*src + 1) == '=') ||
        (buffer[0] == '<' && *(*src + 1) == '=') ||
        (buffer[0] == '>' && *(*src + 1) == '=')) {
        buffer[1] = *(*src + 1);
        *src += 2;
    } else {
        *src += 1;
    }

    add_token(TOKEN_OPERATOR, buffer);
}

void read_symbol(const char **src) {
    char buffer[2] = {0};
    buffer[0] = *(*src)++;
    add_token(TOKEN_SYMBOL, buffer);
}

void print_tokens() {
    for (int i = 0; i < token_count; i++) {
        printf("[%d] Type: %d, Text: '%s'\n", i, tokens[i].type, tokens[i].text);
    }
}

Token *tokenize(const char *src, int *out_token_count) {
    tokens = malloc(sizeof(Token) * MAX_TOKENS);
    if (!tokens) {
        fprintf(stderr, "Memory allocation failed for tokens array\n");
        exit(1);
    }
    token_count = 0;

    while (*src) {
        skip_whitespace(&src);

        if (isalpha(*src) || *src == '_') {
            read_identifier(&src);
        } else if (isdigit(*src)) {
            read_number(&src);
        } else if (*src == '"') {
            read_string(&src);
        } else if (strchr("=+-*/!<>&|", *src)) {
            read_operator(&src);
        } else if (strchr(";(){},", *src)) {
            read_symbol(&src);
        } else if (*src == '#') {
            while (*src && *src != '\n') src++;  // skip comment
        } else {
            printf("Unrecognized character: %c\n", *src);
            if (*src == '\n') line_number++;
            src++;
        }
    }

    add_token(TOKEN_EOF, "EOF");

    if (out_token_count) *out_token_count = token_count;
    return tokens;
}

void free_tokens(Token *tokens, int token_count) {
    for (int i = 0; i < token_count; i++) {
        free(tokens[i].text);
    }
    free(tokens);
}
/*
int main(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <sourcefile>\n", argv[0]);
        return 1;
    }

    char *program = read_file(argv[1]);

    tokenize(program);
    print_tokens();
    free(program);
    return 0;
}
*/