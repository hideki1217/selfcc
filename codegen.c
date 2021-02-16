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

// endregion
int min(int x,int y){return x>y?y:x;}
void *copy(char* s, char* str,int len){strncpy(s,str,len);s[len]='\0';}

Type *gen_lval(Node *node){
    if(node->kind == ND_LVAR){
        LVar* var=((VarNode *)node)->var;
        printf("    mov rax, rbp\n");
        printf("    sub rax, %d\n",var->offset);
        printf("    push rax\n");
        return var->type;
    }
    if(node->kind == ND_DEREF){
        Type *tp=gen(((BinaryNode*)node)->lhs);
        if(tp->ty==PRIM)
            error("ポインタ型でない変数を参照できません。");
        return tp->ptr_to;
    }
    Type *tp=gen(node);
    if (tp->ty==PRIM)
        error("代入の左辺値が変数もしくはポインタの参照ではありません");
    return tp;
}

Type *gen(Node *node){
    int lcount;
    Type *tp;
    switch(node->kind){
    case ND_ROOTINE:
        {
            RootineNode *rootine=(RootineNode*)node;
            char name[rootine->namelen+1];
            copy(name,rootine->name,rootine->namelen);
            printf("%s:\n",name);
            // プロローグ
            // local変数分の領域を確保　
            printf("    push rbp\n");
            printf("    mov rbp, rsp\n");
            printf("    sub rsp, %d\n",rootine->total_offset);

            //引数の読み込み(ローカル変数として)
            int count=0;
            for(VarNode *var=rootine->arg;
                var;
                var=(VarNode*)var->base.next,count++)
            {
                Type *vtp=gen_lval((Node*)var);
                printf("    pop rax\n");
                if(count<6)    
                    printf("    mov %s [rax], %s\n",sizeoption(vtp),registry_for_arg(vtp,count));
                else
                    printf("    mov %s [rax], [rbp+%d]\n",sizeoption(vtp),(count-6)*8+16);//ここ微妙
            }

            tp=gen(rootine->block);

            // エピローグ
            // 最後の式の結果がRAXに残っているのでそれが返り値になる
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");

            return tp;
        }
    case ND_NUM:
        {
            printf("    push %d\n",((NumNode *)node)->val);
            return ((NumNode *)node)->type;
        }
    case ND_LVAR:
        {
            tp=gen_lval(node);
            printf("    pop rax\n");
            printf("    mov %s, %s [rax]\n",rax(tp),sizeoption(tp));
            printf("    push rax\n");
            return tp;
        }
    case ND_ASSIGN:
        {
            Type *ltp=gen_lval(((BinaryNode*)node)->lhs);
            Type *rtp=gen(((BinaryNode*)node)->rhs);

            if(!equal(ltp,rtp))
                error("左辺値と右辺の評価値の型が違います。");
            printf("    pop rdi\n");
            printf("    pop rax\n");
            printf("    mov %s [rax], %s\n",sizeoption(ltp),rdi(ltp));
            printf("    push rdi\n");

            return ltp;
        }
    case ND_RETURN:
        {
            tp=gen(((BinaryNode*)node)->lhs);
            //エピローグ
            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return tp;
        }
    case ND_IF:
        {
            lcount=Lcount++;
            gen(((CondNode*)node)->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    push rax\n");
            printf("    je .Lend%d\n",lcount);
            tp=gen(((CondNode*)node)->T);
            printf(".Lend%d:\n",lcount);
            return tp;
        }
    case ND_IFEL:
        {
            lcount=Lcount++;
            gen(((CondNode*)node)->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    push rax\n");
            printf("    je .Lelse%d\n",lcount);
            Type *ttp=gen(((CondNode*)node)->T);
            printf("    jmp .Lend%d\n",lcount);
            printf(".Lelse%d:\n",lcount);
            Type *ftp=gen(((CondNode*)node)->F);
            printf(".Lend%d:\n",lcount);
            return ttp;// TODO: ここの型管理は未完成
        }
    case ND_WHILE:
        {
            lcount=Lcount++;
            printf(".Lbegin%d:\n",lcount);
            gen(((CondNode*)node)->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    push rax\n");
            printf("    je .Lend%d\n",lcount);
            tp=gen(((CondNode*)node)->T);
            printf("    jmp .Lbegin%d\n",lcount);
            printf(".Lend%d:\n",lcount);
            return tp;
        }
    case ND_FOR:
        {
            lcount=Lcount++;
            gen(((ForNode*)node)->init);
            printf(".Lbegin%d:\n",lcount);
            gen(((ForNode*)node)->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    push rax\n");
            printf("    je .Lend%d\n",lcount);
            gen(((ForNode*)node)->T);
            Type *tp=gen(((ForNode*)node)->update);
            printf("    jmp .Lbegin%d\n",lcount);
            printf(".Lend%d:\n",lcount);
            return tp;
        }
    case ND_BLOCK:
        {
            for(Node *elem=((BlockNode*)node)->block;
                elem;
                elem=elem->next)
            {
                tp=gen(elem);
                printf("    pop rax\n");
            }
            printf("    push rax\n");
            return tp;
        }
    case ND_FUNCTION:
        {
            FuncNode* fnode=((FuncNode*)node);
            int argcount=0;
            for(Node *elem=fnode->arg;
                elem; 
                elem=elem->next,argcount++);
            Type *mem[argcount];
            int i=argcount-1;
            for(Node *elem=fnode->arg;
                elem;
                elem=elem->next,i--)
            {
                tp=gen(elem);
                if(i<6)mem[i]=tp;
            }
            for(int i=0;i<min(6,argcount);i++){
                printf("    pop rax\n");
                printf("    mov %s, %s\n",registry_for_arg(mem[i],i),rax(mem[i]));
            }

            char str[fnode->namelen+1];
            copy(str,fnode->funcname,fnode->namelen);
            printf("    call %s\n",str);
            printf("    push rax\n");
            return find_type_from_name("int");// TODO: とりあえずの案
        }
    case ND_ADDR:
        {
            tp=gen_lval(((BinaryNode*)node)->lhs);
            return new_Pointer(tp);
        }
    case ND_DEREF:
        {
            tp=gen(((BinaryNode*)node)->lhs);
            if(tp->ty==PRIM)
                error("ポインタ型でない変数を参照できません。");
            printf("    pop rax\n");
            printf("    mov %s, %s [rax]\n",rax(tp->ptr_to),sizeoption(tp->ptr_to));
            printf("    push rax\n");
            return tp->ptr_to;
        }
    }
    
    Type *ltp=gen(((BinaryNode*)node)->lhs);
    Type *rtp=gen(((BinaryNode*)node)->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    Type *type_int=find_type_from_name("int");
    tp=type_int;
    switch (node->kind)
    {
    case ND_ADD:
        if(ltp->ty==PTR && rtp->ty==PTR)
            error("ポインタ型同士の足し算は定義されていません");
        if(ltp==type_int && rtp->ty==PTR){
            printf("    imul rax, %d\n",rtp->ptr_to->size);
            tp=rtp;
        }
        if(ltp->ty==PTR && rtp==type_int){
            printf("    imul rdi, %d\n",ltp->ptr_to->size);
            tp=ltp;
        }
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        if(ltp->ty==PTR && rtp->ty==PTR)
            error("ポインタ型同士の足し算は定義されていません");
        if(ltp==type_int && rtp->ty==PTR){
            printf("    imul rax, %d\n",rtp->ptr_to->size);
            tp=rtp;
        }
        if(ltp->ty==PTR && rtp==type_int){
            printf("    imul rdi, %d\n",ltp->ptr_to->size);
            tp=ltp;
        }
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        if(ltp==type_int && rtp==type_int)
            printf("    imul rax, rdi\n");
        else 
            error("数値型同士以外の掛け算は定義されていません");
        tp=type_int;
        break;
    case ND_DIV:
        if(ltp==type_int && rtp==type_int){
            printf("    cqo\n");
            printf("    idiv rdi\n");
        }else 
            error("数値型同士以外の割り算は定義されていません");
        tp=type_int;
        break;
    case ND_GRT:
        if(ltp==type_int && rtp==type_int){
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
        }else
            error("数値型同士以外の比較演算子は定義されていません");
        tp=type_int;
        break;
    case ND_GOE:
        if(ltp==type_int && rtp==type_int){
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
        }else
            error("数値型同士以外の比較演算子は定義されていません");
        tp=type_int;
        break;
    case ND_EQU:
        if(ltp!=rtp)
            error("型が違うもの同士を比較することはできません");
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        tp=type_int;
        break;
    case ND_NEQ:
        if(ltp!=rtp)
            error("型が違うもの同士を比較することはできません");
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        tp=type_int;
        break;
    default:
        break;
    }

    printf("    push rax\n");

    return tp;
}