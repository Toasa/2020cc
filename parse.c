#include <stdio.h>
#include <stdlib.h>
#include "token.h"
#include "parse.h"

int equal_string(char *s1, char *s2) {
    do {
        if (*s1 != *s2) { return 0; }
        s1++;
        s2++;
    } while (*s1 && *s2);
    return 1;
}

Node *new_node(NodeKind nk, int val) {
    Node *n = calloc(1, sizeof(Node));
    n->nk = nk;
    n->lhs = NULL;
    n->rhs = NULL;
    n->val = val;
    return n;
}

// 現在検査中のトークン
Token *token;

void next_token() {
    token = token->next;
}

void expect(TokenKind tk) {
    if (token->tk != tk) {
        printf("expect token error\n");
        exit(1);
    }
    next_token();
}

Node *parse_expr();

Node *parse_primary() {
    Node *n;
    if (token->tk == TK_LPARENT) {
        next_token();
        n = parse_expr();
        expect(TK_RPARENT);
    } else {
        n = new_node(ND_NUM, token->val);
        next_token();
    }
    return n;
}

Node *parse_mul() {
    Node *lhs = parse_primary();

    while (equal_string("*", token->str) || equal_string("/", token->str)) {
        if (equal_string("*", token->str)) {
            next_token();
            Node *rhs = parse_primary();
            Node *n = new_node(ND_MUL, 0);
            n->lhs = lhs;
            n->rhs = rhs;
            lhs = n;
        } else {
            next_token();
            Node *rhs = parse_primary();
            Node *n = new_node(ND_DIV, 0);
            n->lhs = lhs;
            n->rhs = rhs;
            lhs = n;
        }
    }

    return lhs;
}

Node *parse_expr() {
    Node *lhs = parse_mul();

    while (equal_string("+", token->str) || equal_string("-", token->str)) {
        if (equal_string("+", token->str)) {
            next_token();
            Node *rhs = parse_mul();
            Node *n = new_node(ND_ADD, 0);
            n->lhs = lhs;
            n->rhs = rhs;
            lhs = n;
        } else {
            next_token();
            Node *rhs = parse_mul();
            Node *n = new_node(ND_SUB, 0);
            n->lhs = lhs;
            n->rhs = rhs;
            lhs = n;
        }
    }
    
    return lhs;
}

Node *parse(Token *t) {
    token = t;
    Node *root = parse_expr();
    return root;
}