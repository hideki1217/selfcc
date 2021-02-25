#pragma once

#include <stdbool.h>

#include "collections.h"

typedef struct Token Token;

typedef struct Type Type;

typedef struct Var Var;
typedef struct CVar CVar;
typedef struct CStr CStr;

typedef struct LVar LVar;
typedef struct GVar GVar;

typedef struct Rootine Rootine;

typedef struct Node Node;
typedef struct BlockNode BlockNode;
typedef struct BinaryNode BinaryNode;
typedef struct UnaryNode UnaryNode;
typedef struct NumNode NumNode;
typedef struct ConstNode ConstNode;
typedef struct CondNode CondNode;
typedef struct ForNode ForNode;
typedef struct FuncNode FuncNode;
typedef struct VarNode VarNode;
typedef struct RootineNode RootineNode;
typedef struct VarInitNode VarInitNode;

typedef struct CC_Map_for_LVar CC_Map_for_LVar;





extern Token *tkstream;
extern char *user_input;
extern char *filename;

extern int Lcount;// if,for,whileのjmp先を命名するために使用
extern int LCcount;// string 等のconst値を指定するために使用 これをさわるのはCVarのみ

extern Node *code;
extern Node *nullNode;

extern CC_AVLTree *type_tree;

extern CC_Map_for_LVar *locals;
extern CC_AVLTree *globals;

extern CC_Vector *constants;


typedef enum { TK_RESERVED, TK_STRING, TK_IDENT, TK_NUM, TK_EOF } TokenKind;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};
Token *new_Token(TokenKind kind, Token *cur, char *str, int len);



// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

//文字が期待する文字列にに当てはまるなら、trueを返して一つ進める
bool consume(char *op);
bool check(char *op);

Token *consume_hard();
//変数値があるか確認
Token *expect_var_not_proceed();

Token *consume_string();

// identを一つ返し一つ進める
Token *consume_ident();
Token *expect_ident();
Token *expect_var();
//変数値があるか確認し、進めない
Token *expect_var_not_proceed();

//文字が期待する文字列に当てはまらないならエラーを吐く
void expect(char op);

//型名がなければエラーを吐く
Type *expect_type();

//トークンが数であればそれを出力し、トークンを一つ進める。
int expect_number();

bool token_ismutch(Token *token, char *str, int len);

bool at_eof();

Token *tokenize(char *p);

typedef enum {
    ND_ADD,     //"+"
    ND_SUB,     //"-"
    ND_MUL,     //"*"
    ND_DIV,     //"/"
    ND_EQU,     //"=="
    ND_NEQ,     //"!="
    ND_GRT,     //"<"
    ND_GOE,     //"<="
    ND_ASSIGN,  //"="
    ND_LVAR,    //ローカル変数
    ND_GVAR,    //グローバル変数
    ND_RETURN,
    ND_IF,
    ND_IFEL,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,  //ブロック
    ND_FUNCTION,
    ND_ROOTINE,
    ND_ADDR,   //'&'
    ND_DEREF,  //'*'
    ND_INCRE,//'++'
    ND_DECRE,//'--'
    ND_SIZEOF,
    ND_SET,       // NDをまとめるもの
    ND_LVARINIT,  //ローカル変数を初期化
    ND_GVARINIT,  //グローバル変数を初期化
    ND_NUM,
    ND_STR
} NodeKind;

struct Node {
    NodeKind kind;
    Node *next;
    Type *type;
};
Node *new_Node(NodeKind kind);
struct BinaryNode {
    Node base;
    Node *lhs;
    Node *rhs;
};
BinaryNode *new_BinaryNode(NodeKind kind, Node *lhs, Node *rhs);
struct UnaryNode{
    Node base;
    Node *target;
};
UnaryNode *new_UnaryNode(NodeKind kind, Node *target);
struct NumNode {
    Node base;
    int val;  // for ND_NUM
    Type *type;
};
NumNode *new_NumNode(int val);
struct ConstNode{
    Node base;
    int const_id;// 文字列識別用ID
};
ConstNode *new_ConstNode(CVar *var);
struct CondNode {
    Node base;
    Node *T;
    Node *F;
    Node *cond;
};
CondNode *new_CondNode(NodeKind kind, Node *cond, Node *T, Node *F);
struct ForNode {
    Node base;
    Node *init;
    Node *T;
    Node *cond;
    Node *update;
};
ForNode *new_ForNode(Node *init, Node *cond, Node *update, Node *A);
struct FuncNode {
    Node base;
    char *funcname;
    int namelen;
    Node *arg;
};
FuncNode *new_FuncNode(char *funcname, int namelen);
struct VarNode {
    Node base;
    Var *var;  // for ND_VAR
};
VarNode *new_VarNode(Var *var);
struct RootineNode {
    Node base;
    Node *block;
    VarNode *arg;
    char *name;
    int namelen;
    int total_offset;
    char *moldname;
    int moldlen;
};
RootineNode *new_RootineNode(char *name, int len, char *moldname, int moldlen);
struct BlockNode {
    Node base;
    Node *block;
};
BlockNode *new_BlockNode();
struct VarInitNode {
    Node base;
    Var *var;
    Node *value;
};
VarInitNode *new_VarInitNode(Var *var, Node *value);



//文法部
void program();
Node *rootine();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Type *type_assign(Node *node);

void gen_lval(Node *node,bool push);
void gen(Node *node,bool push);

//型を管理
typedef enum {
    TY_VOID,
    TY_INT,
    TY_CHAR,
    TY_LONG,
    TY_FLOAT,
    TY_DOUBLE,
    TY_PTR,
    TY_ARRAY,
    TY_STRUCT
} TypeKind;
struct Type {
    TypeKind kind;
    Type *ptr_to;
    char *name;
    int len;
    int size;
    int array_len;
};

Type *new_PrimType(TypeKind kind, char *name, int len, int size);
Type *new_Pointer(Type *base);
Type *new_Array(Type *base, int length);
Type *new_Struct(Type *bases);
Type *find_type(Token *token);
Type *find_type_from_name(char *name);
bool equal(Type *l, Type *r);
bool check_Type();
bool isArrayorPtr(Type *type);
bool isNum(Type *type);
bool isInteger(Type *type);
bool isAssignable(Type *l, Type *r);

Type *consume_Type();
int make_memorysize(Type *type);


void Initialize_type_tree();



//変数を管理
typedef enum { LOCAL, GLOBAL } Var_kind;
struct Var {
    Var_kind kind;
    char *name;
    int len;
    Type *type;
};
Var *find_Var(Token *token);
Var *get_Var(Token *token);

// 定数を管理
struct CVar{
    Var base;
    int LC_id;
};
struct CStr{
    CVar base;
    char* text;
    int len;
};
CVar *new_CStr(char* text,int len);


//ローカル変数
struct LVar {
    Var base;
    int offset;  // RBPからのoffset、による型のサイズ
};
LVar *add_lvar(Token *token, Type *type);



struct CC_Map_for_LVar {
    CC_AVLTree base;
    int offset;
};
CC_Map_for_LVar *cc_map_for_var_new();
void cc_map_for_var_delete(CC_Map_for_LVar *map);
void cc_map_for_var_clear(CC_Map_for_LVar *map);
void cc_map_for_var_add(CC_Map_for_LVar *map, char *key, int key_len,
                        LVar *item);
void *cc_map_for_var_search(CC_Map_for_LVar *map, char *key, int key_len);
bool cc_map_for_var_empty(CC_Map_for_LVar *map);

//グローバル変数
struct GVar {
    Var base;
};
GVar *add_gvar(Token *token, Type *type);


//関数を管理
struct Rootine {
    RootineNode *node;
    char *name;
    int namelen;
    Type *type;
};

char *registry_for_arg(Type *tp, int i);
char *sizeoption(Type *tp);
char *movzx2rax(Type *tp);// 符号拡張しない
char *movsx2rax(Type *tp);// 符号拡張する(あっているか怪しい)
char *movzx2rdi(Type *tp);
char *movsx2rdi(Type *tp);

// 符号拡張あり
char *movsx(Type *tp);
// 符号拡張なし
char *movzx(Type *tp);

char *rax(Type *tp);
char *rbx(Type *tp);
char *rcx(Type *tp);
char *rdx(Type *tp);
char *rsi(Type *tp);
char *rdi(Type *tp);
char *r8(Type *tp);
char *r9(Type *tp);
char *r10(Type *tp);
char *r11(Type *tp);
char *r12(Type *tp);
char *r13(Type *tp);
char *r14(Type *tp);
