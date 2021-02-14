#include "selfcc.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

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
VarNode *new_VarNode(int offset){
    VarNode *node=calloc(1,sizeof(VarNode));
    node->base.kind=ND_LVAR;
    node->offset=offset;
}

//文法部
Node *code[100];
int Lcount=0;
Node *nullNode;
char* pointargReg[6];

void program(){
    int i=0;
    while(!at_eof()){
        code[i++]=stmt();
    }
    code[i]=NULL;
}
Node *stmt(){
    if(consume("{")){
        Node *node=new_Node(ND_BLOCK);
        Node *now=node;
        while(!consume("}")){
            Node *next=stmt();
            now->next=next;
            now=next;
        }
        now->next=NULL;
        return node; 
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
                Node *arg=primary();
                arg->next=args;
                args=arg;
            }
            node->arg=args;
            return (Node*)node;
        }
        else{//変数の場合
            VarNode *node=new_VarNode(0);

            LVar *var=find_lvar(token);
            if(var){
                node->offset=var->offset;
                return (Node*)node;
            }else{
                var=calloc(1,sizeof(LVar));
                var->next=locals;
                var->name=token->str;
                var->len=token->len;
                var->offset=locals? 
                    locals->offset + 8 
                    : 8;
                node->offset=var->offset;
                locals=var;
            }
            return (Node*)node;
        }
    }

    return (Node*)new_NumNode(expect_number());
}

// endregion
int min(int x,int y){return x>y?y:x;}
void gen_lval(Node *node){
    if(node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n",((VarNode *)node)->offset);
    printf("    push rax\n");
}

void gen(Node *node){
    int lcount;
    switch(node->kind){
    case ND_NUM:
        printf("    push %d\n",((NumNode *)node)->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(((BinaryNode*)node)->lhs);
        gen(((BinaryNode*)node)->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");

        return;
    case ND_RETURN:
        gen(((BinaryNode*)node)->lhs);
        //エピローグ
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_IF:
        lcount=Lcount++;
        gen(((CondNode*)node)->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n",lcount);
        gen(((CondNode*)node)->T);
        printf(".Lend%d:\n",lcount);
        return;
    case ND_IFEL:
        lcount=Lcount++;
        gen(((CondNode*)node)->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lelse%d\n",lcount);
        gen(((CondNode*)node)->T);
        printf("    jmp .Lend%d\n",lcount);
        printf(".Lelse%d:\n",lcount);
        gen(((CondNode*)node)->F);
        printf(".Lend%d:\n",lcount);
        return;
    case ND_WHILE:
        lcount=Lcount++;
        printf(".Lbegin%d:\n",lcount);
        gen(((CondNode*)node)->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n",lcount);
        gen(((CondNode*)node)->T);
        printf("    jmp .Lbegin%d\n",lcount);
        printf(".Lend%d:\n",lcount);
        return;
    case ND_FOR:
        lcount=Lcount++;
        gen(((ForNode*)node)->init);
        printf(".Lbegin%d:\n",lcount);
        gen(((ForNode*)node)->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n",lcount);
        gen(((ForNode*)node)->T);
        gen(((ForNode*)node)->update);
        printf("    jmp .Lbegin%d\n",lcount);
        printf(".Lend%d:\n",lcount);
        return;
    case ND_BLOCK:
        for(Node *elem=node->next;
            elem;
            elem=elem->next)
        {
            gen(elem);
            printf("    pop rax\n");
        }
        printf("    push rax\n");
        return;
    case ND_FUNCTION:
        {
            FuncNode* fnode=((FuncNode*)node);
            int argcount=0;
            for(Node *elem=fnode->arg;
                elem;
                elem=elem->next,argcount++)
            {
                gen(elem);
            }
            for(int i=0;i<min(6,argcount);i++){
                printf("    pop rax\n");
                printf("    mov %s, rax\n",pointargReg[i]);
            }

            char str[fnode->namelen+1];
            strncpy(str,fnode->funcname,fnode->namelen);
            str[fnode->namelen] ='\0';
            printf("    call %s\n",str);
            printf("    push rax\n");
            return;
        }
    }
    
    gen(((BinaryNode*)node)->lhs);
    gen(((BinaryNode*)node)->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_GRT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_GOE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_EQU:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NEQ:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    default:
        break;
    }

    printf("    push rax\n");
}