#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//
// token.c
//
typedef enum {
    TK_NUM,
    TK_RESERVED,
    TK_IDENT,
    TK_LPARENT,   // (
    TK_RPARENT,   // )
    TK_LBRACE,    // {
    TK_RBRACE,    // }
    TK_LBRACKET,  // [
    TK_RBRACKET,  // ]
    TK_SEMICOLON, // ;
    TK_COLON,     // :
    TK_QUEST,     // ?
    TK_COMMA,     // ,
    TK_PERIOD,    // .
    TK_ARROW,     // ->
    TK_TYPE,      // 'void', 'char', 'shor', 'int', 'long'
                  // 'struct', 'union', '_Bool', 'enum'
    TK_STORAGE,   // 'typedef'
    TK_STR,
    TK_EOF,
} TokenKind;

typedef struct Token {
    TokenKind tk;
    int val;
    char *str;
    int str_len;        // tkがTK_STRの場合に使用
    struct Token *next;
} Token;

Token *tokenize(char *input);

//
// parce.c
//
const int MAX_FUNC_NUM;

typedef enum {
    ND_NUM,
    ND_LVAR,
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_REM,       // %
    ND_LSHIFT,    // <<
    ND_RSHIFT,    // >>
    ND_DEREF,     // *
    ND_ADDR,      // &
    ND_COMMA,     // ,
    ND_EQ,        // ==
    ND_NE,        // !=
    ND_LT,        // <
    ND_LE,        // <=
    ND_ASSIGN,    // =
    ND_COND,      // ?:
    ND_NOT,       // !
    ND_BITNOT,    // ~
    ND_BITAND,    // &
    ND_BITOR,     // |
    ND_BITXOR,    // ^
    ND_LOGAND,    // &&
    ND_LOGOR,     // ||
    ND_RETURN,    // return
    ND_CONTINUE,  // continue
    ND_GOTO,      // goto
    ND_LABEL,     // labeled statement
    ND_BREAK,     // break
    ND_IF,        // if
    ND_WHILE,     // while
    ND_FOR,       // for
    ND_BLOCK,     // { ... }
    ND_SWITCH,    // switch
    ND_CASE,      // case
    ND_CALL,      // function call
    ND_DECL,      // local variable declaration
    ND_STMT_EXPR, // statement expression
    ND_MEMBER,    // . (struct member access)
    ND_CAST,      // (type-name) expr
} NodeKind;

typedef enum {
    CHAR,
    SHORT,
    INT,
    LONG,
    PTR,
    ARRAY,
    STRUCT,
    UNION,
    VOID,
    BOOL,
    ENUM,
} TypeKind;

typedef struct Member Member;

typedef struct Type {
    TypeKind tk;
    size_t size;
    int align;
    struct Type *base; // base type of array or pointer
    char *name;

    size_t arr_size;   // the number of elements

    Member *member;    // struct
} Type;

typedef struct VarAttr {
    bool is_typedef;
    bool is_static;
} VarAttr;

struct Member {
    char *name;
    Type *type;
    Member *next;
    int offset;
};

typedef enum VarKind {
    LOCAL,
    GLOBAL,
    ARG,
    MEMBER,
    ENUMMEMBER,
} VarKind;

typedef struct Var {
    VarKind vk;
    char *name;
    Type *type;
    int is_global;
    VarAttr attr;

    // local variable
    int offset;

    // global variable
    char *str;
    int str_len;

    // value of each enum members
    int enum_val;
} Var;

typedef struct VarNode {
    Var data;
    struct VarNode *next;
} VarNode;

typedef struct Tag {
    char *name;
    Type *type; // STRUCT, UNION, or ENUM
    struct Tag *next;
} Tag;

typedef struct Scope {
    struct Scope *high;
    VarNode *lvar_head;
    Tag *tag_head;

    size_t depth;
    size_t total_var_size;
} Scope;

typedef struct FuncData {
    char *name;
    struct Node *body;
    Type *return_type;

    int args_num;
    struct Node *args; // nkがND_CALLの場合に使う。次の引数には`next`メンバからアクセスする。

    size_t stack_frame_size;
    Scope *toplevel_scope;

    bool is_static;
} FuncData;

typedef struct Node {
    NodeKind nk;
    struct Node *lhs;
    struct Node *rhs;
    int val;

    Type *ty;

    Var var;            // nkがND_LVAR, ND_DECLの場合に使う
    FuncData *func;     // nkがND_CALLの場合に使う

    Member *member;     // nkがND_MEMBERの場合に使う

    struct Node *init;  // nkがND_FORの場合に使う
    struct Node *cond;  // nkがND_IF, ND_WHILE, ND_FOR, ND_SWITCHの場合に使う
    struct Node *then;  // nkがND_IF, ND_WHILE, ND_FOR, ND_SWITCH, ND_CONDの場合に使う
    struct Node *alt;   // nkがND_IF, ND_CONDの場合に使う
    struct Node *expr;  // nkがND_RETURN, ND_DEREF, ND_ADDR,
                        // ND_MEMBER, ND_CAST, ND_NOT, ND_BITNOT,
                        // の場合に使う
    struct Node *post;  // nkがND_FORの場合に使う
    struct Node *next;  // nkがND_BLOCK, ND_CALL, ND_DECLの場合に使う
    struct Node *block; // nkがND_BLOCK, ND_STMT_EXPRの場合に使う
    struct Node *stmt;  // nkがND_LABEL, ND_CASEの場合に使う

    // Goto or labeled statement
    char *label_name;

    // switch-cases
    struct Node *case_next;
    struct Node *default_case;
    int case_label;
    int case_end_label;
} Node;

typedef struct Program {
    struct FuncData **funcs;
    VarNode *gvars;
} Program;

Node *new_cast_node(Node *expr, Type *ty);
Program *parse(Token *t);

//
// type.c
//
extern Type *char_t;
extern Type *short_t;
extern Type *int_t;
extern Type *long_t;
extern Type *void_t;
extern Type *bool_t;
Type *new_type(TypeKind tk, Type *base, int align);
int align_to(int n, int align);
Type *pointer_to(Type *base);
Type *array_of(Type *base, int len);
int is_pointer(Type *t);
int is_integer(Type *t);
Type *copy_type(Type *ty);
void add_type(Node *n);

//
// codegen.c
//
void gen(Program *p);

//
// util.c
//
int equal_strings(char *s1, char *s2);
void assert(int result, char *fmt, ...);
void error(char *fmt, ...);
