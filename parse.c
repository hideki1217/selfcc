#include "selfcc.h"

#include<stdlib.h>

Node *new_Node(NodeKind kind){
    Node *node=calloc(1,sizeof(Node));
    node->kind=kind;
    return node;
}
BinaryNode *new_BinaryNode(NodeKind kind,Node *lhs,Node* rhs){
    BinaryNode *node=calloc(1,sizeof(BinaryNode));
    node->base.kind=kind;
    node->lhs=lhs;
    node->rhs=rhs;
    return node;
}
NumNode *new_NumNode(int val){
    NumNode *node=calloc(1,sizeof(NumNode));
    node->base.kind=ND_NUM;
    node->val=val;
    node->type=find_type_from_name("int");
    return node;
}
CondNode *new_CondNode(NodeKind kind,Node *cond,Node *T,Node *F){
    CondNode *node=calloc(1,sizeof(CondNode));
    node->base.kind=kind;
    node->cond=cond;
    node->T=T;
    node->F=F;
    return node;
}
ForNode *new_ForNode(Node *init,Node *cond,Node *update,Node *T){
    ForNode *node=calloc(1,sizeof(ForNode));
    node->base.kind=ND_FOR;
    node->init=init;
    node->cond=cond;
    node->update=update;
    node->T=T;
    return node;
}
FuncNode *new_FuncNode(char *funcname,int namelen){
    FuncNode *node=calloc(1,sizeof(FuncNode));
    node->base.kind=ND_FUNCTION;
    node->funcname=funcname;
    node->namelen=namelen;
    return node;
}
VarNode *new_VarNode(LVar *var){
    VarNode *node=calloc(1,sizeof(VarNode));
    node->base.kind=ND_LVAR;
    node->var=var;
}
RootineNode *new_RootineNode(char *name,int len,char* moldname,int moldlen){
    RootineNode *node=calloc(1,sizeof(RootineNode));
    node->base.kind=ND_ROOTINE;
    node->name=name;
    node->namelen=len;
    node->moldname=moldname;
    node->moldlen=moldlen;
    return node;
}
BlockNode *new_BlockNode(){
    BlockNode *node=calloc(1,sizeof(BlockNode));
    node->base.kind=ND_BLOCK;
    return node;
}

//文法部
Node *code;
int Lcount=0;
Node *nullNode;

void program(){
    int i=0;
    Node head;
    head.next=NULL;
    Node *node=&head;
    while(!at_eof()){
        node->next=rootine();
        node=node->next;
    }
    node->next=NULL;
    code=head.next;
}
Node *rootine(){
    locals=NULL;//ローカル変数をrootineごとにリセット
    Type *type=expect_type();
    Token *token=expect_ident();
    if(consume("(")){//関数定義
        RootineNode *node=new_RootineNode(token->str,token->len,type->name,type->len);

        Node anker;
        anker.next=NULL;
        Node *top=&anker;
        while(!consume(")")){
            consume(",");

            type=expect_type();
            token=expect_var();
            LVar *var=add_lvar(token,type);
            locals=var;

            top->next=(Node*)new_VarNode(var);
            top=top->next;
        }
        node->arg=(VarNode*)(anker.next);
        node->block=stmt();
        if(locals)node->total_offset=locals->offset;

        return (Node*)node;
    }
    else{//グローバル変数
    }
}
Node *stmt(){
    if(consume("{")){
        BlockNode *node=new_BlockNode();

        Node anker;
        anker.next=NULL;
        Node *now=&anker;
        while(!consume("}")){
            Node *next=stmt();
            now->next=next;
            now=next;
        }
        now->next=NULL;
        node->block=anker.next;

        return (Node*)node; 
    }
    if(consume("return")){
        BinaryNode *node=new_BinaryNode(ND_RETURN,expr(),NULL);
        expect(';');
        return (Node*)node;
    }
    if(consume("if")){
        expect('(');
        Node *condition=expr();
        expect(')');
        Node *A=stmt();

        CondNode *node;
        if(consume("else")){
            node=new_CondNode(ND_IFEL,condition,A,stmt());
        }
        else{
            node=new_CondNode(ND_IF,condition,A,NULL);
        }
        return (Node*)node;
    }
    if(consume("while")){
        expect('(');
        Node *condition=expr();
        expect(')');
        Node *A=stmt();

        CondNode *node=new_CondNode(ND_WHILE,condition,A,NULL);
        return (Node*)node;
    }
    if(consume("for")){
        expect('(');
        Node *init=check(";")?nullNode:expr();
        expect(';');
        Node *cond=check(";")?nullNode:expr();
        expect(';');
        Node *update=check(")")?nullNode:expr();
        expect(')');

        ForNode *node=new_ForNode(init,cond,update,stmt());
        
        return (Node*)node;
    }
    Node *node=expr();
    expect(';');
    return node;
}
Node *expr(){
    if (check_Type()){
        Type *type=consume_Type();
        Token* token=expect_var();
        LVar *var=add_lvar(token,type);
        return (Node*)new_VarNode(var);
    }
    return assign();
}
Node *assign(){
    Node *node=equality();
    if(consume("=")){
        node=(Node*)new_BinaryNode(ND_ASSIGN,node,assign());
    }
    return node;
}
Node *equality(){
    Node *node=relational();

    while(1){
        if(consume("==")){
            node=(Node*)new_BinaryNode(ND_EQU,node,relational());
        }
        else if(consume("!=")){
            node=(Node*)new_BinaryNode(ND_NEQ,node,relational());
        }
        else
            return node;
    }
}
Node *relational(){
    Node *node=add();

    while(1){
        if(consume(">=")){
            node=(Node*)new_BinaryNode(ND_GOE,add(),node);
        }
        else if(consume("<=")){
            node=(Node*)new_BinaryNode(ND_GOE,node,add());
        }
        else if(consume(">")){
            node=(Node*)new_BinaryNode(ND_GRT,add(),node);
        }
        else if(consume("<")){
            node=(Node*)new_BinaryNode(ND_GRT,node,add());
        }
        else
            return node;
    }
}
Node *add(){
    Node *node=mul();

    while(1){
        if(consume("+")){
            node=(Node*)new_BinaryNode(ND_ADD,node,mul());
        }
        else if(consume("-")){
            node=(Node*)new_BinaryNode(ND_SUB,node,mul());
        }
        else
            return node;
    }
}
Node *mul(){
    Node *node=unary();

    while(1){
        if (consume("*"))
            node=(Node*)new_BinaryNode(ND_MUL,node,unary());
        else if (consume("/"))
            node=(Node*)new_BinaryNode(ND_DIV,node,unary());
        else
            return node;
    }
}
Node *unary(){
    if (consume("+"))
        return primary();
    if (consume("-"))
        return (Node*)new_BinaryNode(ND_SUB,(Node*)new_NumNode(0),primary());
    if(consume("&"))
        return (Node*)new_BinaryNode(ND_ADDR,unary(),NULL);
    if(consume("*"))
        return (Node*)new_BinaryNode(ND_DEREF,unary(),NULL);
    if(consume("sizeof")){
        return (Node*)new_NumNode(type_assign(unary())->size);
    }
    return primary();
}
Node *primary(){
    if (consume("(")){
        Node *node=expr();
        expect(')');
        return node;
    }

    Token *token=consume_ident();
    if (token){
        if(consume("(")){//関数呼び出しの場合
            FuncNode *node=new_FuncNode(token->str,token->len);

            Node *args=NULL;
            while(!consume(")")){
                consume(",");
                Node *arg=add();
                arg->next=args;
                args=arg;
            }
            node->arg=args;

            return (Node*)node;
        }
        else{//変数の場合
            LVar *var=get_lvar(token);
            return (Node*)new_VarNode(var);
        }
    }

    return (Node*)new_NumNode(expect_number());
}
