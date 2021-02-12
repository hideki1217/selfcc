#include<stdbool.h>

typedef enum{
    TK_RESERVED,
    TK_RETURN,
    TK_IDENT,
    TK_NUM,
    TK_EOF
}TokenKind;
typedef struct Token Token;
struct Token{
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};
Token *new_Token(TokenKind kind,Token* cur,char *str,int len);

extern Token *token;
extern char *user_input;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...);
void error_at(char *loc,char *fmt, ...);

//文字が期待する文字列にに当てはまるなら、trueを返して一つ進める
bool consume(char *op);

Token *consume_ident();
bool consume_return();

//文字が期待する文字列に当てはまらないならエラーを吐く
void expect(char op);

//トークンが数であればそれを出力し、トークンを一つ進める。
int expect_number();

bool at_eof();

Token *tokenize(char *p);

typedef struct LVar LVar;
struct LVar{
    LVar *next;//次の変数かNULL
    char *name;//変数名
    int len;   //名前の長さ
    int offset;//RBPからのoffset
};
extern LVar *locals;

LVar *find_lvar(Token *token);


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
    ND_NUM
}NodeKind;
typedef struct Node Node;
struct Node{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;// for ND_NUM
    int offset;// for ND_LVAR
};
Node *new_Node(NodeKind kind,Node *lhs,Node* rhs);
Node *new_Node_num(int val);

extern Node *code[100];

//文法部
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void gen_lval(Node *node);
void gen(Node *node);