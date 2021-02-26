#include <stdlib.h>

#include "selfcc.h"


void set_Node(Node* node,NodeKind kind){
    node->kind = kind;
}
void set_BinaryNode(BinaryNode *node,NodeKind kind, Node *lhs, Node *rhs){
    node->base.kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
}
void set_UnaryNode(UnaryNode *node,NodeKind kind, Node *target){
    node->base.kind=kind;
    node->target = target;
}
void set_NumNode(NumNode* node,int val){
    node->base.kind = ND_NUM;
    node->val = val;
    node->type = find_type_from_name("int");
}
void set_ConstNode(ConstNode* nd,CVar *var){
    nd->const_id=var->LC_id;
    nd->base.kind=ND_STR;
    nd->base.type=var->base.type;
}
void set_CondNode(CondNode* node,NodeKind kind, Node *cond, Node *T, Node *F){
    node->base.kind = kind;
    node->cond = cond;
    node->T = T;
    node->F = F;
}
void set_ForNode(ForNode* node,Node *init, Node *cond, Node *update, Node *T){
    node->base.kind = ND_FOR;
    node->init = init;
    node->cond = cond;
    node->update = update;
    node->T = T;
}
void set_FuncNode(FuncNode* node,char *funcname, int namelen){
    node->base.kind = ND_FUNCTION;
    node->funcname = funcname;
    node->namelen = namelen;
}
void set_VarNode(VarNode *node,Var *var){
    if (var->kind == LOCAL)
        node->base.kind = ND_LVAR;
    else
        node->base.kind = ND_GVAR;
    node->var = var;
}
void set_RootineNode(RootineNode* node,char *name, int len, char *moldname, int moldlen){
    node->base.kind = ND_ROOTINE;
    node->name = name;
    node->namelen = len;
    node->moldname = moldname;
    node->moldlen = moldlen;
}
void set_BlockNode(BlockNode* node){
    node->base.kind = ND_BLOCK;
}
void set_VarInitNode(VarInitNode* node,Var *var, Node *value){
    if (var->kind == LOCAL)
        node->base.kind = ND_LVARINIT;
    else
        node->base.kind = ND_GVARINIT;
    node->var = var;
    node->value = value;
}

Node *new_Node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    set_Node(node,kind);
    return node;
}
BinaryNode *new_BinaryNode(NodeKind kind, Node *lhs, Node *rhs) {
    BinaryNode *node = calloc(1, sizeof(BinaryNode));
    set_BinaryNode(node,kind,lhs,rhs);
    return node;
}
UnaryNode *new_UnaryNode(NodeKind kind, Node *target){
    UnaryNode *node= calloc(1,sizeof(UnaryNode));
    set_UnaryNode(node,kind,target);
    return node;
}
NumNode *new_NumNode(int val) {
    NumNode *node = calloc(1, sizeof(NumNode));
    set_NumNode(node,val);
    return node;
}
ConstNode *new_ConstNode(CVar *var){
    ConstNode *nd=calloc(1,sizeof(ConstNode));
    set_ConstNode(nd,var);
    return nd;
}
CondNode *new_CondNode(NodeKind kind, Node *cond, Node *T, Node *F) {
    CondNode *node = calloc(1, sizeof(CondNode));
    set_CondNode(node,kind,cond,T,F);
    return node;
}
ForNode *new_ForNode(Node *init, Node *cond, Node *update, Node *T) {
    ForNode *node = calloc(1, sizeof(ForNode));
    set_ForNode(node,init,cond,update,T);
    return node;
}
FuncNode *new_FuncNode(char *funcname, int namelen) {
    FuncNode *node = calloc(1, sizeof(FuncNode));
    set_FuncNode(node,funcname,namelen);
    return node;
}
VarNode *new_VarNode(Var *var) {
    VarNode *node = calloc(1, sizeof(VarNode));
    set_VarNode(node,var);
    return node;
}
RootineNode *new_RootineNode(char *name, int len, char *moldname, int moldlen) {
    RootineNode *node = calloc(1, sizeof(RootineNode));
    set_RootineNode(node,name,len, moldname, moldlen);
    return node;
}
BlockNode *new_BlockNode() {
    BlockNode *node = calloc(1, sizeof(BlockNode));
    set_BlockNode( node);
    return node;
}
VarInitNode *new_VarInitNode(Var *var, Node *value) {
    VarInitNode *node = calloc(1, sizeof(VarInitNode));
    set_VarInitNode(node,var,value);
    return node;
}

//文法部
Node *code;
int Lcount = 0;
int LCcount = 0;
Node *nullNode;

void program() {
    Node head;
    head.next = NULL;
    Node *node = &head;
    while (!at_eof()) {
        node->next = rootine();
        node = node->next;
    }
    node->next = NULL;
    code = head.next;
}
Node *rootine() {
    cc_map_for_var_clear(
        locals);  // ローカル変数をrootineごとにリセット
                  // TODO: mapをqueueにぶちこんで管理するBlockに入るたびにpushしていき出るときにpop
    Type *type = expect_type();
    Token *token = expect_ident();
    if (consume("(")) {  // 関数定義
        RootineNode *node =
            new_RootineNode(token->str, token->len, type->name, type->len);

        Node anker;
        anker.next = NULL;
        Node *top = &anker;
        while (!consume(")")) {
            consume(",");

            type = expect_type();
            token = expect_var();
            LVar *var = add_lvar(token, type);
            cc_map_for_var_add(locals, var->base.name, var->base.len, var);

            top->next = (Node *)new_VarNode((Var *)var);
            top = top->next;
        }
        node->arg = (VarNode *)(anker.next);
        node->block = stmt();
        node->total_offset = locals->offset;

        return (Node *)node;
    } else {  //グローバル変数
        if (consume("[")) {
            int size = expect_number();
            expect(']');
            type = new_Array(type, size);
        }
        GVar *var = add_gvar(token, type);
        Node *value = NULL;
        if (consume("=")) {
            error_at(
                token->str,
                "グローバル変数の初期化は未対応です");  // TODO: グローバル変数の初期化未対応
        }
        expect(';');
        return (Node *)new_VarInitNode((Var *)var, value);
    }
}
Node *stmt() {
    if (consume("{")) {
        BlockNode *node = new_BlockNode();

        Node *set = new_Node(ND_SET);
        node->block = set;

        while (!consume("}")) {
            Node *next = stmt();
            set->next = next;
            set = next;
        }
        set->next = NULL;

        return (Node *)node;
    }
    if (consume("return")) {
        BinaryNode *node = new_BinaryNode(ND_RETURN, expr(), NULL);
        expect(';');
        return (Node *)node;
    }
    if (consume("if")) {
        expect('(');
        Node *condition = expr();
        expect(')');
        Node *A = stmt();

        CondNode *node;
        if (consume("else")) {
            node = new_CondNode(ND_IFEL, condition, A, stmt());
        } else {
            node = new_CondNode(ND_IF, condition, A, NULL);
        }
        return (Node *)node;
    }
    if (consume("while")) {
        expect('(');
        Node *condition = expr();
        expect(')');
        Node *A = stmt();

        CondNode *node = new_CondNode(ND_WHILE, condition, A, NULL);
        return (Node *)node;
    }
    if (consume("for")) {
        expect('(');
        Node *init = check(";") ? nullNode : expr();
        expect(';');
        Node *cond = check(";") ? nullNode : expr();
        expect(';');
        Node *update = check(")") ? nullNode : expr();
        expect(')');

        ForNode *node = new_ForNode(init, cond, update, stmt());

        return (Node *)node;
    }
    Node *node = expr();
    expect(';');
    return node;
}
Node *expr() {
    if (check_Type()) {
        Type *type = consume_Type();

        Token *tk = expect_var_not_proceed();
        if (token_ismutch(tk->next, "[", 1)) {  //配列の場合
            consume_ident();
            consume("[");
            int size =
                expect_number();  // TODO: 定数値を実装し次第それも入れるべし
            expect(']');
            type = new_Array(type, size);
            LVar *var = add_lvar(tk, type);

            return (Node *)new_VarInitNode(
                (Var *)var,
                NULL);  // TODO: 配列初期化じの初期値を受けるならNULLを解除
        } else {  //配列でない場合
            LVar *var = add_lvar(tk, type);
            return assign();
        }
    }
    return assign();
}
Node *assign() {
    Node *node = equality();
    if (consume("=")) 
        return (Node *)new_BinaryNode(ND_ASSIGN, node, assign());
    if(consume("+="))
        return (Node*)new_BinaryNode(ND_ADDASS,node,assign());
    if(consume("-="))
        return (Node*)new_BinaryNode(ND_SUBASS,node,assign());
    if(consume("*="))
        return (Node*)new_BinaryNode(ND_MULASS,node,assign());
    if(consume("/="))
        return (Node*)new_BinaryNode(ND_DIVASS,node,assign());

    return node;
}
Node *equality() {
    Node *node = relational();

    while (1) {
        if (consume("==")) {
            node = (Node *)new_BinaryNode(ND_EQU, node, relational());
        } else if (consume("!=")) {
            node = (Node *)new_BinaryNode(ND_NEQ, node, relational());
        } else
            return node;
    }
}
Node *relational() {
    Node *node = add();

    while (1) {
        if (consume(">=")) {
            node = (Node *)new_BinaryNode(ND_GOE, add(), node);
        } else if (consume("<=")) {
            node = (Node *)new_BinaryNode(ND_GOE, node, add());
        } else if (consume(">")) {
            node = (Node *)new_BinaryNode(ND_GRT, add(), node);
        } else if (consume("<")) {
            node = (Node *)new_BinaryNode(ND_GRT, node, add());
        } else
            return node;
    }
}
Node *add() {
    Node *node = mul();

    while (1) {
        if (consume("+")) {
            node = (Node *)new_BinaryNode(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = (Node *)new_BinaryNode(ND_SUB, node, mul());
        } else
            return node;
    }
}
Node *mul() {
    Node *node = unary();

    while (1) {
        if (consume("*"))
            node = (Node *)new_BinaryNode(ND_MUL, node, unary());
        else if (consume("/"))
            node = (Node *)new_BinaryNode(ND_DIV, node, unary());
        else
            return node;
    }
}
Node *unary() {
    if (consume("+")) return primary();
    if (consume("-"))
        return (Node *)new_BinaryNode(ND_SUB, (Node *)new_NumNode(0),
                                      primary());
    if (consume("&")) return (Node *)new_BinaryNode(ND_ADDR, unary(), NULL);
    if (consume("*")) return (Node *)new_BinaryNode(ND_DEREF, unary(), NULL);
    if (consume("sizeof")) {
        return (Node *)new_NumNode(type_assign(unary())->size);
    }
    if(consume("++")) return (Node*)new_BinaryNode(ND_ADD, (Node *)new_NumNode(1),primary()); // ++x
    if(consume("--")) return (Node*)new_BinaryNode(ND_SUB,primary(), (Node *)new_NumNode(1)); // --x

    Node *node= primary();
    if(consume("++")) return (Node*)new_BinaryNode(ND_INCRE,node,(Node *)new_NumNode(1)); // x++
    if(consume("--")) return (Node*)new_BinaryNode(ND_DECRE,node,(Node *)new_NumNode(1)); // x--

    return node;
}
Node *primary() {
    Node *nd;

    if (consume("(")) {  // ( expr)の場合
        Node *node = expr();
        expect(')');
        nd = node;
    } else {
        Token *tk = consume_hard();
        switch (tk->kind) {
            case TK_NUM: {  // 数値の場合
                nd = (Node *)new_NumNode(tk->val);
                break;
            }
            case TK_STRING:{
                CVar *var=new_CStr(tk->str,tk->len);
                nd = (Node *) new_ConstNode(var);
                cc_vector_add(constants,(void*)var);
                break;
            }
            case TK_IDENT: {
                if (consume("(")) {  // 関数の場合
                    FuncNode *node = new_FuncNode(tk->str, tk->len);
                    Node *args = NULL;
                    while (!consume(")")) {
                        consume(",");
                        Node *arg = add();
                        arg->next = args;
                        args = arg;
                    }
                    node->arg = args;
                    nd = (Node *)node;
                } else {  // 変数の場合
                    Var *var = get_Var(tk);
                    nd = (Node *)new_VarNode(var);
                }
                break;
            }
        }
    }
    if (consume("[")) {  // a[x]の形の時:: a[x] -> *(a+x)
        Node *index = add();
        expect(']');
        nd = (Node *)new_BinaryNode(
            ND_DEREF, (Node *)new_BinaryNode(ND_ADD, nd, index), NULL);
    }

    return nd;
}
