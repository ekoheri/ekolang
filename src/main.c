#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Gagal membuka file");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char *buffer = malloc(length + 1);
    if (!buffer) {
        perror("Gagal alokasi memori");
        exit(1);
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <sourcefile>\n", argv[0]);
        return 1;
    }

    char *source_code = read_file(argv[1]);

    int token_count;
    Token *tokens = tokenize(source_code, &token_count);

    Parser parser;
    parser_init(&parser, tokens, token_count);
    parse_all(&parser);

    free_tokens(tokens, token_count);

    return 0;
}
