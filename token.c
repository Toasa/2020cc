#include "2020cc.h"

int is_digit(char c) {
    return ('0' <= c && c <= '9');
}

int is_char(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

int is_space(char c) {
    return c == ' ';
}

int can_skip(char c) {
    return is_space(c) || c == '\n' || c == '\t';
}

int is_keyword(char *str) {
    char *keywords[] = {
        "return",
        "if",
        "else",
        "while",
        "for",
        "sizeof",
        NULL,
    };
    for (int i = 0; keywords[i] != NULL; i++) {
        if (equal_strings(str, keywords[i])) {
            return 1;
        }
    }
    return 0;
}

int is_type(char *str) {
    char *types[] = {
        "char",
        "short",
        "int",
        "long",
        "struct",
        "union",
        "void",
        NULL,
    };
    for (int i = 0; types[i] != NULL; i++) {
        if (equal_strings(str, types[i])) {
            return 1;
        }
    }
    return 0;
}

Token *new_token(Token *prev, TokenKind tk, int val, char *str) {
    Token *t = calloc(1, sizeof(Token));
    t->tk = tk;
    t->val = val;
    t->str = str;
    prev->next = t;
    return t;
}

void skip(char **input) {
    while (can_skip(**input)) {
        (*input)++;
    }
}

int read_number(char **input) {
    int num = 0;
    while (is_digit(**input)) {
        num = num * 10 + (**input - '0');
        (*input)++;
    }
    return num;
}

// 一文字目はchar、二文字目以降はchar, 数字, `_`のいづれかの文字を読み、
// 新たにメモリを確保した上で文字列を返す
char *read_ident(char **input) {
    char *input_org = *input;

    int str_count = 0;
    if (is_char(**input)) {
        (*input)++;
        str_count++;
    } else {
        return NULL;
    }

    while (is_char(**input) || (**input == '_') || is_digit(**input)) {
        (*input)++;
        str_count++;
    }
    char *str = calloc(1, sizeof(char) * (str_count + 1));
    strncpy(str, input_org, str_count);
    return str;
}

char read_escaped_char(char c) {
    switch (c) {
        case 'a': return '\a';
        case 'b': return '\b';
        case 't': return '\t';
        case 'n': return '\n';
        case 'v': return '\v';
        case 'f': return '\f';
        case 'r': return '\r';
        case 'e': return 27;
        default: return c;
    }
}

Token *new_str_token(Token *cur_token, char **input) {
    char *input_org = *input;

    int str_count = 0;
    while (**input != '"') {
        if (**input == '\\' && *(*input+1) == '"') {
            (*input)++;
            str_count++;
        }
        (*input)++;
        str_count++;
    }

    char *str = calloc(1, sizeof(char) * (str_count + 1));
    int i = 0;
    for (char *c = input_org; *c != '"'; c++) {
        if (*c == '\\') {
            c++;
            str[i] = read_escaped_char(*c);
        } else {
            str[i] = *c;
        }
        i++;
    }

    Token *str_token = new_token(cur_token, TK_STR, 0, str);
    str_token->str_len = str_count + 1;
    return str_token;
}

Token *tokenize(char *input) {
    Token *head_token = calloc(1, sizeof(Token));
    Token *cur_token = head_token;
    
    while (*input) {
        skip(&input);
        if (is_digit(*input)) {
            int num = read_number(&input);
            cur_token = new_token(cur_token, TK_NUM, num, "");
        } else if (is_char(*input)) {
            char *str = read_ident(&input);
            if (is_keyword(str)) {
                cur_token = new_token(cur_token, TK_RESERVED, 0, str);
            } else if (is_type(str)) {
                cur_token = new_token(cur_token, TK_TYPE, 0, str);
            } else {
                // identifier
                cur_token = new_token(cur_token, TK_IDENT, 0, str);
            }
        } else if (*input == '"') {
            input++;
            cur_token = new_str_token(cur_token, &input);
            input++;
        } else if (*input == '+') {
            input++;
            if (*input == '=') {
                input++;
                cur_token = new_token(cur_token, TK_RESERVED, 0, "+=");
            } else if (*input == '+') {
                input++;
                cur_token = new_token(cur_token, TK_RESERVED, 0, "++");
            } else {
                cur_token = new_token(cur_token, TK_RESERVED, 0, "+");
            }
        } else if (*input == '-') {
            input++;
            if (*input == '=') {
                input++;
                cur_token = new_token(cur_token, TK_RESERVED, 0, "-=");
            } else if (*input == '-') {
                input++;
                cur_token = new_token(cur_token, TK_RESERVED, 0, "--");
            } else if (*input == '>') {
                input++;
                cur_token = new_token(cur_token, TK_ARROW, 0, "->");
            } else {
                cur_token = new_token(cur_token, TK_RESERVED, 0, "-");
            }
        } else if (*input == '*') {
            input++;
            if (*input == '=') {
                input++;
                cur_token = new_token(cur_token, TK_RESERVED, 0, "*=");
            } else {
                cur_token = new_token(cur_token, TK_RESERVED, 0, "*");
            }
        } else if (*input == '/') {
            input++;
            if (*input == '=') {
                input++;
                cur_token = new_token(cur_token, TK_RESERVED, 0, "/=");
            } else if (*input == '*') {
                input++;
                while (!(*input == '*' && *(input + 1) == '/')) {
                    input++;
                }
                input += 2;
                continue;
            } else if (*input == '/') {
                while (*input != '\n') {
                    input++;
                }
                input++;
                continue;
            } else {
                cur_token = new_token(cur_token, TK_RESERVED, 0, "/");
            }
        } else if (*input == '%') {
            input++;
            if (*input == '=') {
                input++;
                cur_token = new_token(cur_token, TK_RESERVED, 0, "%=");
            } else {
                cur_token = new_token(cur_token, TK_RESERVED, 0, "%");
            }
        } else if (*input == '&') {
            input++;
            cur_token = new_token(cur_token, TK_RESERVED, 0, "&");
        } else if (*input == ';') {
            input++;
            cur_token = new_token(cur_token, TK_SEMICOLON, 0, ";");
        } else if (*input == ',') {
            input++;
            cur_token = new_token(cur_token, TK_COMMA, 0, ",");
        } else if (*input == '.') {
            input++;
            cur_token = new_token(cur_token, TK_PERIOD, 0, ".");
        } else if (*input == '(') {
            input++;
            cur_token = new_token(cur_token, TK_LPARENT, 0, "(");
        } else if (*input == ')') {
            input++;
            cur_token = new_token(cur_token, TK_RPARENT, 0, ")");
        } else if (*input == '{') {
            input++;
            cur_token = new_token(cur_token, TK_LBRACE, 0, "{");
        } else if (*input == '}') {
            input++;
            cur_token = new_token(cur_token, TK_RBRACE, 0, "}");
        } else if (*input == '[') {
            input++;
            cur_token = new_token(cur_token, TK_LBRACKET, 0, "[");
        } else if (*input == ']') {
            input++;
            cur_token = new_token(cur_token, TK_RBRACKET, 0, "]");
        } else if (*input == '=') {
            input++;
            if (*input == '=') {
                input++;
                cur_token = new_token(cur_token, TK_RESERVED, 0, "==");
            } else {
                cur_token = new_token(cur_token, TK_RESERVED, 0, "=");
            }
        } else if (*input == '!') {
            input++;
            if (*input == '=') {
                input++;
                cur_token = new_token(cur_token, TK_RESERVED, 0, "!=");
            }
        } else if (*input == '<') {
            input++;
            if (*input == '=') {
                input++;
                cur_token = new_token(cur_token, TK_RESERVED, 0, "<=");
            } else if (*input == '<') {
                input++;
                if (*input == '=') {
                    input++;
                    cur_token = new_token(cur_token, TK_RESERVED, 0, "<<=");
                } else {
                    cur_token = new_token(cur_token, TK_RESERVED, 0, "<<");
                }
            } else {
                cur_token = new_token(cur_token, TK_RESERVED, 0, "<");
            }
        } else if (*input == '>') {
            input++;
            if (*input == '=') {
                input++;
                cur_token = new_token(cur_token, TK_RESERVED, 0, ">=");
            } else if (*input == '>') {
                input++;
                if (*input == '=') {
                    input++;
                    cur_token = new_token(cur_token, TK_RESERVED, 0, ">>=");
                } else {
                    cur_token = new_token(cur_token, TK_RESERVED, 0, ">>");
                }
            } else {
                cur_token = new_token(cur_token, TK_RESERVED, 0, ">");
            }
        } else {
            break;
        }
    }
    cur_token = new_token(cur_token, TK_EOF, 0, "");
    return head_token->next;
}

