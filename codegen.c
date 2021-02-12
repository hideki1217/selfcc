#include "selfcc.h"
#include<stdio.h>
#include<stdlib.h>

Node *new_Node(NodeKind kind,Node *lhs,Node* rhs){
    Node *node=calloc(1,sizeof(Node));
    node->kind=kind;
    node->lhs=lhs;
    node->rhs=rhs;
    return node;
}
Node *new_Node_num(int val){
    Node *node=calloc(1,sizeof(Node));
    node->kind=ND_NUM;
    node->val=val;
    return node;
}

//文法部
Node *code[100];

void program(){
    int i=0;
    while(!at_eof()){
        code[i++]=stmt();
    }
    code[i]=NULL;
}
Node *stmt(){
    if(consume_return()){
        Node *node=new_Node(ND_RETURN,expr(),NULL);
        expect(';');
        return node;
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
        node=new_Node(ND_ASSIGN,node,assign());
    }
    return node;
}
Node *equality(){
    Node *node=relational();

    while(1){
        if(consume("==")){
            node=new_Node(ND_EQU,node,relational());
        }
        else if(consume("!=")){
            node=new_Node(ND_NEQ,node,relational());
        }
        else
            return node;
    }
}
Node *relational(){
    Node *node=add();

    while(1){
        if(consume(">=")){
            node=new_Node(ND_GOE,add(),node);
        }
        else if(consume("<=")){
            node=new_Node(ND_GOE,node,add());
        }
        else if(consume(">")){
            node=new_Node(ND_GRT,add(),node);
        }
        else if(consume("<")){
            node=new_Node(ND_GRT,node,add());
        }
        else
            return node;
    }
}
Node *add(){
    Node *node=mul();

    while(1){
        if(consume("+")){
            node=new_Node(ND_ADD,node,mul());
        }
        else if(consume("-")){
            node=new_Node(ND_SUB,node,mul());
        }
        else
            return node;
    }
}
Node *mul(){
    Node *node=unary();

    while(1){
        if (consume("*"))
            node=new_Node(ND_MUL,node,unary());
        else if (consume("/"))
            node=new_Node(ND_DIV,node,unary());
        else
            return node;
    }
}
Node *unary(){
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_Node(ND_SUB,new_Node_num(0),primary());
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
        Node *node=calloc(1,sizeof(Node));
        node->kind=ND_LVAR;

        LVar *var=find_lvar(token);
        if(var){
            node->offset=var->offset;
            return node;
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
        return node;
    }

    return new_Node_num(expect_number());
}

// endregion
void gen_lval(Node *node){
    if(node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");
    
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n",node->offset);
    printf("    push rax\n");
}

void gen(Node *node){
    switch(node->kind){
    case ND_NUM:
        printf("    push %d\n",node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");

        return;
    case ND_RETURN:
        gen(node->lhs);
        //エピローグ
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    }
    
    gen(node->lhs);
    gen(node->rhs);

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