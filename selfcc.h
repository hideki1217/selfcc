#pragma once

#include <stdbool.h>

#include "collections.h"
#include "preprocesser.h"
#include "token.h"
#include "type.h"
#include "vector.h"

typedef struct Var Var;
typedef struct CVar CVar;
typedef struct CStr CStr;

typedef struct LVar LVar;
typedef struct SVar SVar;
typedef struct GVar GVar;
typedef struct ExVar ExVar;

typedef struct Rootine Rootine;

typedef struct Node Node;
typedef struct BlockNode BlockNode;
typedef struct BinaryNode BinaryNode;
typedef struct UnaryNode UnaryNode;
typedef struct NumNode NumNode;
typedef struct FloatNode FloatNode;
typedef struct CharNode CharNode;
typedef struct EnumNode EnumNode;
typedef struct ConstNode ConstNode;
typedef struct CastNode CastNode;
typedef struct CondNode CondNode;
typedef struct ForNode ForNode;
typedef struct CallNode CallNode;
typedef struct VarNode VarNode;
typedef struct RootineNode RootineNode;
typedef struct VarInitNode VarInitNode;
typedef struct LabelNode LabelNode;
typedef struct OffsetNode OffsetNode;

typedef struct LVar_Manager LVar_Manager;
typedef struct Map_for_LVar Map_for_LVar;

typedef int flag_n;

typedef enum StorageMode StorageMode;

#define ARG_MAX 30

//////////////////////////// グローバル変数
extern Token *tkstream;
extern char *user_input;
extern char *filename;

extern int Lcount;  // if,for,whileのjmp先を命名するために使用
extern int
    LCcount;  // string 等のconst値を指定するために使用 これをさわるのはCVarのみ

extern Node *code;
extern Node *nullNode;

extern LVar_Manager *locals;
extern CC_AVLTree *globals;
extern CC_BidList *global_list;
extern CC_AVLTree *externs;

extern CC_BidList *constants;

//////////////////////////////////

// エラーを報告するための関数
// printfと同じ引数を取る
void error_at(char *loc, char *fmt, ...);
void error_here(bool flag, char *fmt, ...);

/** opのtokenであるか確認。進める*/
Token *consume(char *op);
/** 消費したtokenを一つ戻す。 */
void unconsume();
/** opのTokenであるか確認。進めない*/
Token *check(char *op);
/** 一つ先がopのTokenであるか確認。進めない*/
Token *check_ahead(char *s);
void expect(char *op);
void _expect(char *op, Token **token);
Token *consume_ident();
Token *expect_ident();
Token *_expect_ident(Token **tk);
Token *expect_var();
bool check_Type();
Token *consume_Type(Type **tp);
int expect_integer();
Token *consume_integer();
Token *consume_float();
Token *consume_char();
Token *consume_enum();
Token *consume_string();
Token *check_ident();
Token *check_integer();
Token *check_float();
Token *check_char();
Token *check_enum();
Token *check_string();
/** EOFトークンならtrueを返す*/
bool at_eof();

TkSequence *tokenize(char *p);

typedef enum {
    ND_ADD,     //"+"
    ND_SUB,     //"-"
    ND_MUL,     //"*"
    ND_DIV,     //"/"
    ND_OR,      //"|"
    ND_XOR,     //"^"
    ND_AND,     //"&"
    ND_NOT,     //"~"
    ND_MOD,     //"%"
    ND_RSHFT,   //">>"
    ND_LSHFT,   //"<<"
    ND_EQU,     //"=="
    ND_NEQ,     //"!="
    ND_GRT,     //"<"
    ND_GOE,     //"<="
    ND_LGCOR,   //"||" TODO
    ND_LGCAND,  //"&&" TODO
    ND_LGCNOT,  //"!" TODO
    ND_ASSIGN,  //"="
    ND_LVAR,    //ローカル変数
    ND_GVAR,    //グローバル変数
    ND_RETURN,
    ND_GOTO,  // TODO
    ND_BREAK,
    ND_CONTINUE,
    ND_LABEL,    // ** : ... TODO
    ND_CASE,     // case ** : ... TODO
    ND_DEFAULT,  // defalut: ... TODO
    ND_IF,
    ND_IFEL,
    ND_WHILE,
    ND_DOWHILE,
    ND_FOR,
    ND_SWITCH,  // TODO
    ND_BLOCK,   //ブロック
    ND_CAST,    // TODO
    ND_CALL,
    ND_ROOTINE,
    ND_ADDR,    // '&'
    ND_DEREF,   // '*'
    ND_INCRE,   // '++'
    ND_DECRE,   // '--'
    ND_ACCESS,  // '.'
    ND_ADDASS,  // '+='
    ND_SUBASS,  // '-='
    ND_MULASS,  // '*='
    ND_DIVASS,  // '/='
    ND_MODASS,  // '%='
    ND_LSHASS,  // '<<='
    ND_RSHASS,  // '>>='
    ND_ANDASS,  // '&='
    ND_ORASS,   // '|='
    ND_XORASS,  // '^='
    ND_SIZEOF,
    ND_SET,       // NDをまとめるもの
    ND_LVARINIT,  // ローカル変数を初期化
    ND_GVARINIT,  // グローバル変数を初期化
    ND_ENUM,      // TODO
    ND_INT,
    ND_FLOAT,  // TODO
    ND_CHAR,   // TODO
    ND_STR,
    ND_NULL  // 何もしないノード
} NodeKind;

NodeKind pairOf(NodeKind kind);

bool IsVarNode(Node *nd);

struct Node {
    NodeKind kind;
    Node *next;
    Type *type;
    Token *pos;
};
Node *new_Node(NodeKind kind);
void set_Node(Node *nd, NodeKind kind);
struct BinaryNode {
    Node base;
    Node *lhs;
    Node *rhs;
};
BinaryNode *new_BinaryNode(NodeKind kind, Node *lhs, Node *rhs);
void set_BinaryNode(BinaryNode *nd, NodeKind kind, Node *lhs, Node *rhs);
struct UnaryNode {
    Node base;
    Node *target;
};
UnaryNode *new_UnaryNode(NodeKind kind, Node *target);
void set_UnaryNode(UnaryNode *nd, NodeKind kind, Node *target);
struct NumNode {
    Node base;
    int val;  // for ND_INT
};
NumNode *new_NumNode(int val);
void set_NumNode(NumNode *nd, int val);
struct FloatNode {
    Node base;
    double val;
};
FloatNode *new_FloatNode(float val);
void set_FloatNode(FloatNode *node, float val);
struct CharNode {
    Node base;
    char val;
};
CharNode *new_CharNode(char val);
void set_CharNode(CharNode *node, char val);
struct EnumNode {
    Node base;
    char *name;
    int len;
};
EnumNode *new_EnumNode(char *name, int len);
void set_EnumNode(EnumNode *node, char *name, int len);
struct ConstNode {
    Node base;
    int const_id;  // 定数価識別用ID
};
ConstNode *new_ConstNode(CVar *var);
void set_ConstNode(ConstNode *nd, CVar *var);
struct CastNode {
    Node base;
    Node *target;
    Type *cast;
};
CastNode *new_CastNode(Type *cast, Node *target);
void set_CastNode(CastNode *node, Type *cast, Node *target);
struct CondNode {
    Node base;
    int index;
    Node *T;
    Node *F;
    Node *cond;
};
CondNode *new_CondNode(NodeKind kind, Node *cond, Node *T, Node *F);
void set_CondNode(CondNode *nd, NodeKind kind, Node *cond, Node *T, Node *F);
struct ForNode {
    Node base;
    int index;
    Node *init;
    Node *T;
    Node *cond;
    Node *update;
};
ForNode *new_ForNode(Node *init, Node *cond, Node *update, Node *A);
void set_ForNode(ForNode *nd, Node *init, Node *cond, Node *update, Node *A);
struct CallNode {
    Node base;
    Node *func;
    Node *arg;
};
CallNode *new_CallNode(Node *func);
void set_CallNode(CallNode *nd, Node *func);
struct VarNode {
    Node base;
    Var *var;  // for ND_VAR
};
VarNode *new_VarNode(Var *var);
void set_VarNode(VarNode *nd, Var *var);
struct RootineNode {
    Node base;
    Node *block;
    VarNode *arg;
    Var *func;
    int total_offset;
};
RootineNode *new_RootineNode(Var *var, VarNode *args, Node *block);
void set_RootineNode(RootineNode *nd, Var *var, VarNode *args, Node *block);
struct BlockNode {
    Node base;
    Node *block;
};
BlockNode *new_BlockNode(NodeKind kind);
void set_BlockNode(BlockNode *nd, NodeKind kind);
struct VarInitNode {
    Node base;
    Var *var;
    Node *value;
};
VarInitNode *new_VarInitNode(Var *var, Node *value);
void set_VarInitNode(VarInitNode *nd, Var *var, Node *value);
struct LabelNode {
    Node base;
    int jumpTo;
};
LabelNode *new_LabelNode(NodeKind kind, int index);
void set_LabelNode(LabelNode *node, NodeKind kind, int index);
struct OffsetNode{
    Node base;
    Node *origin;
    union {
        int offset;
        String string;
    } tag;
};
OffsetNode *new_OffsetNode(NodeKind kind,Node *origin,char *tag_name,int namelen);
void set_OffsetNode(OffsetNode *node,NodeKind kind,Node *origin,char *tag_name,int namelen);



//文法部
void initialize_parser();

Node *translation_unit();
Node *external_declaration();
/**
 * @brief  グローバルでの宣言
 * @note
 * @retval 宣言によるNode
 */
Node *global_declaration();
/*
param: mode = typedef等のフラグ
param: base = ベースの型
RET: false => 一回も更新されなかった
*/
bool declaration_specifier(StorageMode *mode, TypeModel *model);
/*なければSM_NONE*/
StorageMode storage_specifier();
/*
定義済みの型の名前。
存在しなければNULL
*/
bool type_specifier(TypeModel *model);
/**
 * @brief ベースの型を期待する構文
 * @note
 * @param  **tp: ベースの型
 * @param  errorExpected: エラーを期待するか
 * @retval false=>tokenを消費していない
 */
bool specifier_qualifier(TypeModel *model, bool errorExpected);

/**
 * @brief  抽象な型名を読む
 * @note
 * @param  isCheck: マッチしなければNULLを返すか？
 * @retval 読んだ型(もし何もなければvoid)
 */
Type *type_naming(bool isCheck);
/*識別子を含まない宣言*/
void abstract_declarator(TypeModel *model);
void declarator(TypeModel *model, Token **ident);
/*識別子を含む宣言。識別子を返す。なければNULL*/
Token *direct_declarator(TypeModel *base);

/*RET: 2進数表示でabとすると
a=1 => const
b=1 => volatile */
bool type_qualifier(flag_n *flag);
/**
 * @brief  ローカルでの宣言
 * @note
 * @param asExpr: 式と並列に扱うか?
 * @retval 宣言によるNode
 */
Node *local_declaration(bool asExpr);

void struct_declaration(TypeModel *model);
void struct_declarator(TypeModel *model, Token **ident);

void enum_declaration();

Node *initilizer();

Node *constant_expr();
Node *condition_expr();
Node *logical_or_expr();
Node *logical_and_expr();
Node *or_expr();
Node *xor_expr();
Node *and_expression();
Node *equality_expr();
Node *relational_expr();
Node *shift_expr();
Node *add_expr();
Node *mul_expr();
Node *cast_expr();
Node *unary_expr();
Node *postfix_expr();
Node *primary_expr();
Node *constant();
Node *expression();
Node *assignment_expr();
Node *statement();
Node *compound_stmt(bool hasScope);
Node *labeled_stmt();
Node *expression_stmt();
Node *selection_stmt();
Node *iteration_stmt();
Node *jump_stmt();

/**
 * @brief  関数定義になりえるか確認
 * @note
 * @param  *tp: 対象の型
 * @retval 関数定義になれればtrue
 */
bool CanbeFuncDef(Type *tp);

enum StorageMode {
    SM_NONE,
    SM_AUTO,
    SM_REGISTER,
    SM_STATIC,
    SM_EXTERN,
    SM_TYPEDEF
};

/**
 * @brief  型を割り当てる
 * @note
 * @param  *node: 対象のnode
 * @retval 割り当てられた型
 */
Type *type_assign(Node *node);

/**
 * @brief  nodeから左辺値をraxに残すコードを生成する
 * @note
 * @param  *node: 対象のnode
 * @param  push: 最後にpushしておいてほしいか
 * @retval None
 */
void gen_lval(Node *node, bool push);
/**
 * @brief  nodeからコードを生成する
 * @note
 * @param  *node: 対象のnode
 * @param  push: 最後にpushしてほしいか否か
 * @retval None
 */
void gen(Node *node, bool push);

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
/*
tokenの文字列をつかい、
宣言済みの変数や関数の中から
一致するものを探す。
もしなければ、NULLを返す
*/
Var *find_Var(Token *token);
/*
tokenの文字列をつかい、
宣言済みの変数や関数の中から
一致するものを探す。
もしなければ、エラー終了
*/
Var *get_Var(Token *token);

bool IsExtern(flag_n flag);
bool IsStatic(flag_n flag);
bool IsAuto(flag_n flag);
bool IsRegister(flag_n flag);
flag_n setExtern(flag_n flag, bool tof);
flag_n setStatic(flag_n flag, bool tof);
flag_n setAuto(flag_n flag, bool tof);
flag_n setRegister(flag_n flag, bool tof);
flag_n makeFlag(bool isTypedef, bool isExtern, bool isStatic, bool isAuto,
                bool isRegister);

// 定数を管理
struct CVar {
    Var base;
    int LC_id;
};
struct CStr {
    CVar base;
    char *text;
    int len;
};
CVar *add_CStr(char *text, int len);

//ローカル変数
struct LVar {
    Var base;
    int offset;  // RBPからのoffset、による型のサイズ
};
LVar *add_lvar(Token *token, Type *type);

struct LVar_Manager {
    CC_Queue *queue;
    Map_for_LVar *top;
    int max_offset;
};
LVar_Manager *lvar_manager_new();
void lvar_manager_PushScope(LVar_Manager *manager);
void lvar_manager_PopScope(LVar_Manager *manager);
int lvar_manager_Add(LVar_Manager *manager, char *key, int len, LVar *var);
int lvar_manager_GetOffset(LVar_Manager *manager);
void lvar_manager_SetOffset(LVar_Manager *manager, int offset);
void lvar_manager_Clear(LVar_Manager *manager);
int lvar_manager_GetTotalOffset(LVar_Manager *manager);
LVar *lvar_manager_Find(LVar_Manager *manager, char *key, int len,
                        bool nowScope);

struct Map_for_LVar {
    CC_AVLTree base;
    int offset;
};
Map_for_LVar *map_for_var_new();
void map_for_var_delete(Map_for_LVar *map);
void map_for_var_clear(Map_for_LVar *map);
void map_for_var_add(Map_for_LVar *map, char *key, int key_len, LVar *item);
void *map_for_var_search(Map_for_LVar *map, char *key, int key_len);
bool map_for_var_empty(Map_for_LVar *map);

//グローバル変数
struct GVar {
    Var base;
};
GVar *add_gvar(Token *token, Type *type, bool isStatic);

// 外部変数
struct ExVar {
    Var base;
};
ExVar *add_exvar(Token *token, Type *type);

//関数を管理
struct Rootine {
    RootineNode *node;
    char *name;
    int namelen;
    Type *type;
};

char *registry_for_arg(Type *tp, int i);
char *sizeoption(Type *tp);
char *movzx2rax(Type *tp);  // 符号拡張しない
char *movsx2rax(Type *tp);  // 符号拡張する(あっているか怪しい)
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
