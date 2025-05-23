#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "parser.h"

#define MAX_VARIABLES 256

Variable variables[MAX_VARIABLES];
int var_count = 0;

void parser_init(Parser *p, Token *tokens, int token_count) {
    p->tokens = tokens;
    p->pos = 0;
    p->token_count = token_count;
}

Token *current_token(Parser *p) {
    if (p->pos < p->token_count)
        return &p->tokens[p->pos];
    return NULL;
}

void advance(Parser *p) {
    if (p->pos < p->token_count) p->pos++;
}

int match(Parser *p, TokenType type, const char *text) {
    Token *tok = current_token(p);
    if (!tok) return 0;
    if (tok->type == type && (text == NULL || strcmp(tok->text, text) == 0)) {
        advance(p);
        return 1;
    }
    return 0;
}

// Fungsi untuk menambahkan variabel
void add_variable(const char *name, VarType type, const char *value_str) {
    if (var_count >= MAX_VARIABLES) {
        printf("Error: jumlah variabel melebihi batas\n");
        return;
    }
    variables[var_count].name = strdup(name);
    variables[var_count].type = type;

    switch (type) {
        case TYPE_INT:
            variables[var_count].value.int_val = atoi(value_str);
            break;
        case TYPE_STRING:
            variables[var_count].value.str_val = strdup(value_str);
            break;
        case TYPE_DOUBLE:
            variables[var_count].value.double_val = atof(value_str);
            break;
        default:
            break;
    }
    var_count++;
}

// Fungsi untuk mencari variabel berdasar nama
Variable* find_variable(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return &variables[i];
        }
    }
    return NULL;
}


int evaluate_condition(const char *var_name, const char *op, const char *value_str) {
    Variable *var = find_variable(var_name);
    if (!var) {
        printf("Error: variabel '%s' tidak ditemukan\n", var_name);
        return 0;
    }

    int val = atoi(value_str);  // kita asumsikan yang dibandingkan adalah int dulu

    if (var->type != TYPE_INT) {
        printf("Error: evaluasi hanya mendukung tipe int\n");
        return 0;
    }

    int var_val = var->value.int_val;

    if (strcmp(op, ">") == 0) return var_val > val;
    else if (strcmp(op, "<") == 0) return var_val < val;
    else if (strcmp(op, "==") == 0) return var_val == val;
    else if (strcmp(op, "!=") == 0) return var_val != val;
    else if (strcmp(op, ">=") == 0) return var_val >= val;
    else if (strcmp(op, "<=") == 0) return var_val <= val;

    printf("Error: operator '%s' tidak dikenali\n", op);
    return 0;
}

//Tamabahan untuk ngecek oprrator logika dalam if
int evaluate_logical_expression(Parser *p) {
    int result = 0;
    int current_result;
    char op[3] = "";

    if (match(p, TOKEN_SYMBOL, "(")) {
        current_result = evaluate_logical_expression(p);
        match(p, TOKEN_SYMBOL, ")");
    } else {
        // Ambil ident (variabel), operator, dan nilai
        Token *var_tok = current_token(p);
        if (!match(p, TOKEN_IDENT, NULL)) {
            printf("Error: Diharapkan nama variabel\n");
            return 0;
        }

        Token *op_tok = current_token(p);
        if (!match(p, TOKEN_OPERATOR, NULL)) {
            printf("Error: Diharapkan operator perbandingan\n");
            return 0;
        }

        Token *val_tok = current_token(p);
        if (!match(p, TOKEN_NUMBER, NULL)) {
            printf("Error: Diharapkan nilai numerik\n");
            return 0;
        }

        current_result = evaluate_condition(var_tok->text, op_tok->text, val_tok->text);
    }

    result = current_result;

    while (1) {
        Token *tok = current_token(p);
        if (!tok || tok->type != TOKEN_OPERATOR) break;

        if (strcmp(tok->text, "&&") == 0) {
            match(p, TOKEN_OPERATOR, "&&");

            int rhs = evaluate_logical_expression(p);
            result = result && rhs;
        } else if (strcmp(tok->text, "||") == 0) {
            match(p, TOKEN_OPERATOR, "||");

            int rhs = evaluate_logical_expression(p);
            result = result || rhs;
        } else {
            break;
        }
    }

    return result;
}

void parse_block(Parser *p) {
    if (!match(p, TOKEN_SYMBOL, "{")) {
        printf("Error: '{' diharapkan\n");
        return;
    }
    while (!match(p, TOKEN_SYMBOL, "}")) {
        // Panggil parse_all atau parse_statement di sini
        // Contoh:
        parse_statement(p);
        if (p->pos >= p->token_count) break;
    }
}

void skip_block(Parser *p) {
    // Fungsi untuk melewati blok {...} tanpa parsing isi
    if (!match(p, TOKEN_SYMBOL, "{")) {
        printf("Error: '{' diharapkan untuk skip block\n");
        return;
    }
    int depth = 1;
    while (depth > 0 && p->pos < p->token_count) {
        if (match(p, TOKEN_SYMBOL, "{")) depth++;
        else if (match(p, TOKEN_SYMBOL, "}")) depth--;
        else advance(p);
    }
}

void skip_expression(Parser *p) {
    // Contoh: lewati token sampai tutup kurung ')'
    while (p->pos < p->token_count) {
        if (p->tokens[p->pos].type == TOKEN_SYMBOL && strcmp(p->tokens[p->pos].text, ")") == 0) {
            break;
        }
        p->pos++;
    }
}

int peek_token_is(Parser *p, TokenType type, const char *value) {
    if (p->pos >= p->token_count) {
        return 0;  // sudah melewati token terakhir
    }

    Token *current = &p->tokens[p->pos];

    if (current->type != type) {
        return 0;
    }

    if (value != NULL && strcmp(current->text, value) != 0) {
        return 0;
    }

    return 1;  // token cocok
}

void parse_if_statement(Parser *p) {
    if (!match(p, TOKEN_KEYWORD, "jika")) return;

    if (!match(p, TOKEN_SYMBOL, "(")) {
        printf("Error: '(' diharapkan setelah 'jika'\n");
        return;
    }

    int condition_result = evaluate_logical_expression(p);

    if (!match(p, TOKEN_SYMBOL, ")")) {
        printf("Error: ')' diharapkan setelah kondisi\n");
        return;
    }

    if (condition_result) {
        // Eksekusi blok jika kondisi benar
        parse_block(p);

        // Lewati blok 'lain jika' atau 'lain' setelahnya karena sudah benar
        while (peek_token_is(p, TOKEN_KEYWORD, "lain")) {
            advance(p); // konsumsi 'lain'
            if (peek_token_is(p, TOKEN_KEYWORD, "jika")) {
                // Lewati 'jika' setelah 'lain'
                advance(p);
                // Lewati kondisi dan blok else if
                if (!match(p, TOKEN_SYMBOL, "(")) {
                    printf("Error: '(' diharapkan setelah 'lain jika'\n");
                    return;
                }
                // Lewati kondisi else if (bisa implementasi parse_expression untuk validasi)
                skip_expression(p);
                if (!match(p, TOKEN_SYMBOL, ")")) {
                    printf("Error: ')' diharapkan setelah kondisi 'lain jika'\n");
                    return;
                }
                skip_block(p);
            } else {
                // 'lain' saja, lewati blok else
                skip_block(p);
            }
        }

    } else {
        // Kondisi 'jika' salah, coba cari dan proses 'lain jika' atau 'lain'

        // Lewati blok 'jika' yang salah
        skip_block(p);

        // Cek dan proses 'lain jika' atau 'lain'
        while (peek_token_is(p, TOKEN_KEYWORD, "lain")) {
            advance(p); // konsumsi 'lain'

            if (peek_token_is(p, TOKEN_KEYWORD, "jika")) {
                // 'lain jika' detected
                advance(p); // konsumsi 'jika'

                if (!match(p, TOKEN_SYMBOL, "(")) {
                    printf("Error: '(' diharapkan setelah 'lain jika'\n");
                    return;
                }

                int else_if_condition = evaluate_logical_expression(p);

                if (!match(p, TOKEN_SYMBOL, ")")) {
                    printf("Error: ')' diharapkan setelah kondisi 'lain jika'\n");
                    return;
                }

                if (else_if_condition) {
                    // Eksekusi blok 'lain jika' yang benar
                    parse_block(p);
                    // Selesai, tidak perlu cek lagi 'lain' yang lain
                    break;
                } else {
                    // Lewati blok 'lain jika' yang salah
                    skip_block(p);
                    // Lanjut cek apakah masih ada 'lain' lagi
                }

            } else {
                // 'lain' saja (else)
                parse_block(p);
                break; // selesai setelah else
            }
        }
    }
}

// Contoh parse function call (sama seperti sebelumnya)
void parse_function_call(Parser *p) {
    if (match(p, TOKEN_KEYWORD, "tampil")) {
        if (!match(p, TOKEN_SYMBOL, "(")) {
            printf("Error: '(' expected after 'tampil'\n");
            return;
        }

        Token *arg = current_token(p);
        if (arg && (arg->type == TOKEN_STRING || arg->type == TOKEN_IDENT)) {
            printf("Function arg: %s\n", arg->text);
            advance(p);
        } else {
            printf("Error: argument expected\n");
            return;
        }

        if (match(p, TOKEN_SYMBOL, ",")) {
            arg = current_token(p);
            if (arg && arg->type == TOKEN_IDENT) {
                printf("Function arg: %s\n", arg->text);
                advance(p);
            } else {
                printf("Error: argument setelah koma harus identifier\n");
                return;
            }
        }

        if (!match(p, TOKEN_SYMBOL, ")")) {
            printf("Error: ')' expected\n");
            return;
        }

        if (!match(p, TOKEN_SYMBOL, ";")) {
            printf("Error: ';' expected\n");
            return;
        }

        printf("Fungsi tampil dipanggil\n");
    }
}

void parse_statement(Parser *p) {
    Token *tok = current_token(p);
    if (!tok) return;

    if (tok->type == TOKEN_TYPE) {
        // Tipe data terdeteksi
        printf("Deklarasi variabel bertipe: %s\n", tok->text);
        VarType var_type;

        if (strcmp(tok->text, "string") == 0) var_type = TYPE_STRING;
        else if (strcmp(tok->text, "int") == 0) var_type = TYPE_INT;
        else if (strcmp(tok->text, "double") == 0) var_type = TYPE_DOUBLE;
        else {
            printf("Error: tipe data tidak dikenali\n");
            return;
        }
        advance(p);

        Token *var_name = current_token(p);
        if (var_name && var_name->type == TOKEN_IDENT) {
            printf("Nama variabel: %s\n", var_name->text);
            advance(p);
        } else {
            printf("Error: Nama variabel tidak ditemukan\n");
            return;
        }

        Token *op = current_token(p);
        if (op && op->type == TOKEN_OPERATOR && strcmp(op->text, "=") == 0) {
            advance(p);

            Token *val = current_token(p);
            if (val && (val->type == TOKEN_STRING || val->type == TOKEN_NUMBER)) {
                printf("Nilai awal: %s\n", val->text);
                // *** Tambahkan panggilan add_variable di sini ***
                add_variable(var_name->text, var_type, val->text);
                
                advance(p);
            } else {
                printf("Error: Nilai awal tidak valid\n");
                return;
            }
        } else {
            // Jika tidak ada inisialisasi, beri nilai default
            if (var_type == TYPE_STRING)
                add_variable(var_name->text, var_type, "");
            else if (var_type == TYPE_INT)
                add_variable(var_name->text, var_type, "0");
            else if (var_type == TYPE_DOUBLE)
                add_variable(var_name->text, var_type, "0.0");
        }

        Token *semi = current_token(p);
        if (semi && semi->type == TOKEN_SYMBOL && strcmp(semi->text, ";") == 0) {
            advance(p);  // selesai
        } else {
            printf("Error: titik koma hilang\n");
        }
    }

    else if (tok->type == TOKEN_KEYWORD) {
        if (strcmp(tok->text, "tampil") == 0) {
            parse_function_call(p);
        }
        else if (strcmp(tok->text, "jika") == 0) {
            parse_if_statement(p);  // <--- di sini jika ada if
        }
        else {
            printf("Keyword lain: %s\n", tok->text);
            advance(p);
        }
    } else {
        printf("Token lain: %s\n", tok->text);
        advance(p);
    }
}

void parse_all(Parser *p) {
    while (current_token(p) && current_token(p)->type != TOKEN_EOF) {
        parse_statement(p);
    }
}