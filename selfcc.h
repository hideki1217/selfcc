#include<stdbool.h>

typedef struct Token Token;
typedef struct Type Type;
typedef struct LVar LVar;
typedef struct Rootine Rootine;
typedef struct Node Node;
typedef struct BlockNode BlockNode;
typedef struct BinaryNode BinaryNode;
typedef struct NumNode NumNode;
typedef struct CondNode CondNode;
typedef struct ForNode ForNode;
typedef struct FuncNode FuncNode;
typedef struct VarNode VarNode;
typedef struct RootineNode RootineNode;

typedef enum{
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_EOF
}TokenKind;

struct Token{
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};
Token *new_Token(TokenKind kind,Token* cur,char *str,int len);

extern Token *tkstream;
extern char *user_input;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...);
void error_at(char *loc,char *fmt, ...);

//文字が期待する文字列にに当てはまるなら、trueを返して一つ進める
bool consume(char *op);
bool check(char *op);

//identを一つ返し一つ進める
Token *consume_ident();
Token *expect_ident();
Token *expect_var();

//文字が期待する文字列に当てはまらないならエラーを吐く
void expect(char op);

//型名がなければエラーを吐く
Type* expect_type();

//トークンが数であればそれを出力し、トークンを一つ進める。
int expect_number();

bool at_eof();

Token *tokenize(char *p);


typedef enum{
    ND_ADD,//"+"
    ND_SUB,//"-"
    ND_MUL,//"*"
    ND_DIV,//"/"
    ND_EQU,//"=="
    ND_NEQ,//"!="
    ND_GRT,//"<"
    ND_GOE,//"<="
    ND_ASSIGN,//"="
    ND_LVAR,//変数
    ND_RETURN,
    ND_IF,
    ND_IFEL,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,//ブロック
    ND_FUNCTION,
    ND_ROOTINE,
    ND_ADDR,//'&'
    ND_DEREF,//'*'
    ND_SIZEOF,
    ND_NUM
}NodeKind;


struct Node{
    NodeKind kind;
    Node *next;
    Type *type;
};
Node *new_Node(NodeKind kind);
struct BinaryNode{
    Node base;
    Node *lhs;
    Node *rhs;
};
BinaryNode *new_BinaryNode(NodeKind kind,Node *lhs,Node* rhs);
struct NumNode{
    Node base;
    int val;// for ND_NUM
    Type *type;
};
NumNode *new_NumNode(int val);
struct CondNode{
    Node base;
    Node *T;
    Node *F;
    Node *cond;
};
CondNode *new_CondNode(NodeKind kind,Node *cond,Node *T,Node *F);
struct ForNode{
    Node base;
    Node *init;
    Node *T;
    Node *cond;
    Node *update;
};
ForNode *new_ForNode(Node *init,Node *cond,Node *update,Node *A); 
struct FuncNode{
    Node base;
    char *funcname;
    int namelen;
    Node *arg;
};
FuncNode *new_FuncNode(char *funcname,int namelen);
struct VarNode{
    Node base;
    LVar *var;// for ND_LVAR
};
VarNode *new_VarNode(LVar *var);
struct RootineNode{
    Node base;
    Node *block;
    VarNode *arg;
    char* name;
    int namelen;
    int total_offset;
    char* moldname;
    int moldlen;
};
RootineNode *new_RootineNode(char *name,int len,char*moldname,int moldlen);
struct BlockNode{
    Node base;
    Node *block;
};
BlockNode *new_BlockNode();


extern Node *code;
extern Node *nullNode;

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

Type* type_assign(Node *node);

void gen_lval(Node *node);
void gen(Node *node);

//型を管理
struct Type{
    enum{PRIM,PTR}ty;
    struct Type *ptr_to;
    Type *next;
    char* name;
    int len;
    int size;
};
Type *new_Type(char* name,int len,int size);
Type *new_Pointer(Type *type);
Type *find_type(Token *token);
Type *find_type_from_name(char* name);
bool equal(Type *l,Type *r);
bool check_Type();
Type *consume_Type();
extern Type *types;

//変数を管理
struct LVar{
    LVar *next;//次の変数かNULL
    char *name;//変数名
    int len;   //名前の長さ
    int offset;//RBPからのoffset、による型のサイズ
    Type *type;
};
LVar *new_LVar(Token* token,Type *type);
extern LVar *locals;
extern int Lcount;

LVar *find_lvar(Token *token);
LVar *add_lvar(Token *token,Type *type);
LVar *get_lvar(Token *token);


//関数を管理
struct Rootine{
    Rootine *next;
    RootineNode *node;
    char * name;
    int namelen;
    Type *type;  
};

char* registry_for_arg(Type *tp,int i);
char* sizeoption(Type *tp);
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

