#pragma once

#include <stdbool.h>

#include "collections.h"

typedef struct Token Token;

typedef struct Type Type;

typedef struct Var Var;
typedef struct CVar CVar;
typedef struct CStr CStr;

typedef struct LVar LVar;
typedef struct SVar SVar;
typedef struct GVar GVar;
typedef struct ExVar ExVar;

typedef struct Param Param;
typedef struct Params Params;

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

typedef struct CC_Map_for_LVar CC_Map_for_LVar;

typedef int flag_n;

typedef enum StorageMode StorageMode;

//////////////////////////// グローバル変数
extern Token *tkstream;
extern Token *nowToken;
extern char *user_input;
extern char *filename;

extern int Lcount;  // if,for,whileのjmp先を命名するために使用
extern int
    LCcount;  // string 等のconst値を指定するために使用 これをさわるのはCVarのみ

extern Node *code;
extern Node *nullNode;

extern CC_Map_for_LVar *locals;
extern CC_AVLTree *globals;
extern CC_Vector *global_list;
extern CC_AVLTree *externs;

extern CC_Vector *constants;

//////////////////////////////////

typedef enum {
    TK_RESERVED,
    TK_STRING,
    TK_IDENT,
    TK_NUM,
    TK_EOF,
    TK_FLOAT,
    TK_CHAR,
    TK_ENUM
} TokenKind;

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
void error(char *msg, ...);
void error_at(char *loc, char *fmt, ...);
void error_here(bool flag, char *fmt, ...);

//文字が期待する文字列にに当てはまるなら、trueを返して一つ進める
bool consume(char *op);
bool check(char *op);
//一個先を見る
bool check_ahead(char *s);

Token *consume_hard();
//変数値があるか確認
Token *expect_var_not_proceed();

bool consume_string(Token **tk);

// identを一つ返し一つ進める
bool consume_ident(Token **tk);
Token *expect_ident();
Token *expect_var();
//変数値があるか確認し、進めない
Token *expect_var_not_proceed();

//文字が期待する文字列に当てはまらないならエラーを吐く
void expect(char op);
void expect_str(char* s);

//型名がなければエラーを吐く
Type *expect_type();
bool consume_Type(Type **tp);

//トークンが数であればそれを出力し、トークンを一つ進める。
int expect_integer();
bool consume_integer(Token **tk);
bool consume_float(Token **tk);
bool consume_char(Token **tk);
bool consume_enum(Token **tk);

bool token_ismutch(Token *token, char *str, int len);

bool at_eof();

Token *tokenize(char *p);

typedef enum {
    ND_ADD,     //"+"
    ND_SUB,     //"-"
    ND_MUL,     //"*"
    ND_DIV,     //"/"
    ND_OR,      //"|" TODO
    ND_XOR,     //"^" TODO
    ND_AND,     //"&" TODO
    ND_NOT,     //"~" TODO
    ND_MOD,     //"%" TODO
    ND_RSHFT,   //">>" TODO
    ND_LSHFT,   //"<<" TODO
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
    ND_GOTO, // TODO
    ND_BREAK, // TODO
    ND_CONTINUE, // TODO
    ND_LABEL, // TODO "** : ..."ってやつ
    ND_CASE, // case ** : ...
    ND_DEFAULT, // defalut: ...
    ND_IF,
    ND_IFEL,
    ND_WHILE,
    ND_DOWHILE, // TODO
    ND_FOR,
    ND_SWITCH, // TODO
    ND_BLOCK,  //ブロック
    ND_CAST,   // TODO
    ND_CALL,
    ND_ROOTINE,
    ND_ADDR,    //'&'
    ND_DEREF,   //'*'
    ND_INCRE,   //'++'
    ND_DECRE,   //'--'
    ND_ADDASS,  // '+='
    ND_SUBASS,  // '-='
    ND_MULASS,  //'*='
    ND_DIVASS,  //'/='
    ND_MODASS,  //'%=' TODO
    ND_LSHASS,  //'<<=' TODO
    ND_RSHASS,  //'>>=' TODO
    ND_ANDASS,  //'&=' TODO
    ND_ORASS,   //'|=' TODO
    ND_XORASS,  //'^=' TODO
    ND_SIZEOF,
    ND_SET,       // NDをまとめるもの
    ND_LVARINIT,  //ローカル変数を初期化
    ND_GVARINIT,  //グローバル変数を初期化
    ND_ENUM,      // TODO
    ND_INT,
    ND_FLOAT,  // TODO
    ND_CHAR,   // TODO
    ND_STR,
    ND_NULL // 何もしないノード
} NodeKind;

bool IsVarNode(Node *nd);

struct Node {
    NodeKind kind;
    Node *next;
    Type *type;
    char *pos;
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
    Node *T;
    Node *F;
    Node *cond;
};
CondNode *new_CondNode(NodeKind kind, Node *cond, Node *T, Node *F);
void set_CondNode(CondNode *nd, NodeKind kind, Node *cond, Node *T, Node *F);
struct ForNode {
    Node base;
    Node *init;
    Node *T;
    Node *cond;
    Node *update;
};
ForNode *new_ForNode(Node *init, Node *cond, Node *update, Node *A);
void set_ForNode(ForNode *nd, Node *init, Node *cond, Node *update, Node *A);
struct CallNode {
    Node base;
    Node *ident;
    Node *arg;
};
CallNode *new_CallNode(Node *ident);
void set_CallNode(CallNode *nd,Node *ident);
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
RootineNode *new_RootineNode(Var *var,VarNode *args,Node *block);
void set_RootineNode(RootineNode *nd,Var *var,VarNode *args,Node *block);
struct BlockNode {
    Node base;
    Node *block;
};
BlockNode *new_BlockNode(NodeKind kind);
void set_BlockNode(BlockNode *nd,NodeKind kind);
struct VarInitNode {
    Node base;
    Var *var;
    Node *value;
};
VarInitNode *new_VarInitNode(Var *var, Node *value);
void set_VarInitNode(VarInitNode *nd, Var *var, Node *value);
struct LabelNode{
    Node base;
    char *label;
    int len;
};
LabelNode *new_LabelNode(NodeKind kind,char *label,int len);
void set_LabelNode(LabelNode* node,NodeKind kind,char *label,int len);

//文法部
void program();
Node *rootine();
Node *extern_declaration();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

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
bool declaration_specifier(StorageMode *mode, Type **base);
/*なければSM_NONE*/
StorageMode storage_specifier();
/*
定義済みの型の名前。
存在しなければNULL
*/
Type *type_specifier();

Type *type_name();
/*識別子を含まない宣言*/
void abstract_declarator(Type **base);// 保留
Type *declarator(Type *base,Token **tk);
/*識別子を含む宣言。識別子を返す。なければNULL*/
Token *direct_declarator(Type **base);


/*RET: 2進数表示でabとすると
a=1 => const
b=1 => volatile */
flag_n type_qualifier();
/**
 * @brief  ローカルでの宣言
 * @note   
 * @retval 宣言によるNode
 */
Node *local_declaration();

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
Node *compound_stmt();
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

enum StorageMode{
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
    TY_STRUCT,
    TY_UNION,
    TY_ENUM,
    TY_FUNCTION,
    TY_ALIAS
} TypeKind;
struct Type {
    TypeKind kind;
    Type *ptr_to;
    int size;
    bool isConst;
    bool isVolatile;

    Params *params;
    char *name;
    int len;
    int array_len;
};
/**
 * @brief  type_treeに新たな型名を登録
 * @note   
 * @param  *tp: 登録する型(name,lenを埋めておく必要あり)
 * @retval None
 */
void regist_type(Type *tp);

Type *new_PrimType(TypeKind kind, char *name, int len, int size);
Type *new_Pointer(Type *base);
Type *new_Function(Type *base,Params *arg);
Type *new_Array(Type *base, int length);
Type *new_Struct(Type *bases);
Type *new_Alias(Type *base,char *name,int len);
Type *clone_Type(Type *tp);
Type *find_type(Token *token);
Type *find_type_from_name(char *name);
bool equal(Type *l, Type *r);
bool check_Type();
bool isArrayorPtr(Type *type);
bool isNum(Type *type);
bool isInteger(Type *type);
bool isAssignable(Type *l, Type *r);
bool isLeftsidevalue(Type *tp);
bool isAddSubable(Type *l, Type *r);
bool isMulDivable(Type *l, Type *r);

char *type2str(Type *tp);

int make_memorysize(Type *type);

void Initialize_type_tree();

//パラメータ
typedef enum{ PA_ARG, PA_VAARG} ParamKind;
struct Param{
    ParamKind kind;
    Type *type;
    Param *next;
    char *name;
    int len;
};
struct Params{
    Param *root;
    Param *front;
};
Params *new_Params();
void set_Params(Params *p);
void set_Param(Param *p,Type* tp);
void set_VaArg(Param *p);
/*変数名をセット*/
void params_setIdent(Params *params,char *name,int len);
/*可変長引数出ない引数を足す*/
void params_addParam(Params *p,Type *tp);
/*可変長引数を足す*/
void params_addVaArg(Params *p);
int params_compare(const Params *base,const Params *act);



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
flag_n setExtern(flag_n flag,bool tof);
flag_n setStatic(flag_n flag,bool tof);
flag_n setAuto(flag_n flag,bool tof);
flag_n setRegister(flag_n flag,bool tof);
flag_n makeFlag(bool isTypedef,bool isExtern,bool isStatic,bool isAuto,bool isRegister);


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
GVar *add_gvar(Token *token, Type *type,bool isStatic);

// 外部変数
struct ExVar{
    Var base;
};
ExVar *add_exvar(Token *token,Type *type);

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
