#include "selfcc.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

int min(int x,int y){return x>y?y:x;}
void *copy(char* s, char* str,int len){strncpy(s,str,len);s[len]='\0';}

Type *type_assign(Node *node){
    Type *tp;
    switch(node->kind){
    case ND_ROOTINE:
        {
            RootineNode *rootine=(RootineNode*)node;

            //引数の読み込み(ローカル変数として)
            int count=0;
            for(VarNode *var=rootine->arg;
                var;
                var=(VarNode*)var->base.next,count++)
            {
                Type *vtp=type_assign((Node*)var);// TODO: 関数の情報から引数の型があっているか確認
            }

            tp=type_assign(rootine->block);// TODO: 関数の情報から返り値の型があっているか確認
            
            node->type=tp;
            return tp;
        }
    case ND_NUM:
        {
            tp=((NumNode *)node)->type;
            node->type=tp;
            return tp;
        }
    case ND_LVAR:
        {
            tp=((VarNode*)node)->var->type;
            node->type=tp;
            return tp;
        }
    case ND_ASSIGN:
        {
            Type *ltp=type_assign(((BinaryNode*)node)->lhs);
            Type *rtp=type_assign(((BinaryNode*)node)->rhs);

            if(!equal(ltp,rtp))
                error("左辺値と右辺の評価値の型が違います。");

            node->type=ltp;
            return ltp;
        }
    case ND_RETURN:
        {
            tp=type_assign(((BinaryNode*)node)->lhs);
            node->type=tp;
            return tp;
        }
    case ND_IF:
        {
            tp=type_assign(((CondNode*)node)->cond);// TODO: 条件式がNULLの場合はfalseにする

            tp=type_assign(((CondNode*)node)->T);
            
            node->type=tp;
            return tp;
        }
    case ND_IFEL:
        {
            tp=type_assign(((CondNode*)node)->cond);
            
            Type *ttp=type_assign(((CondNode*)node)->T);
            Type *ftp=type_assign(((CondNode*)node)->F);

            node->type=tp;
            return ttp;// TODO: ここの型管理は未完成
        }
    case ND_WHILE:
        {
            tp=type_assign(((CondNode*)node)->cond);
            
            tp=type_assign(((CondNode*)node)->T);

            node->type=tp;
            return tp;
        }
    case ND_FOR:
        {
            tp=type_assign(((ForNode*)node)->init);
            
            tp=type_assign(((ForNode*)node)->cond);
            
            tp=type_assign(((ForNode*)node)->T);

            type_assign(((ForNode*)node)->update);
           
            node->type=tp;
            return tp;
        }
    case ND_BLOCK:
        {
            Type *ret_type=NULL;
            for(Node *elem=((BlockNode*)node)->block;
                elem;
                elem=elem->next)
            {
                tp=type_assign(elem);
                if(elem->kind==ND_RETURN)
                    break;
            }
            
            node->type=tp;
            return tp;
        }
    case ND_FUNCTION:
        {
            FuncNode* fnode=((FuncNode*)node);
            
            for(Node *elem=fnode->arg;
                elem;
                elem=elem->next)
            {
                tp=type_assign(elem);// TODO:関数の引数チェック
            }

            return find_type_from_name("int");// TODO: 関数の型を返す
        }
    case ND_ADDR:
        {
            tp=type_assign(((BinaryNode*)node)->lhs);
            return new_Pointer(tp);
        }
    case ND_DEREF:
        {
            tp=type_assign(((BinaryNode*)node)->lhs);
            if(tp->ty==PRIM)
                error("ポインタ型でない変数を参照できません。");
            return tp->ptr_to;
        }
    }
    
    Type *ltp=type_assign(((BinaryNode*)node)->lhs);
    Type *rtp=type_assign(((BinaryNode*)node)->rhs);

    Type *type_int=find_type_from_name("int");
    tp=type_int;
    switch (node->kind)
    {
    case ND_ADD:
        if(ltp->ty==PTR && rtp->ty==PTR)
            error("ポインタ型同士の足し算は定義されていません");
        if(ltp==type_int && rtp->ty==PTR)
            tp=rtp;
        if(ltp->ty==PTR && rtp==type_int)
            tp=ltp;
        break;
    case ND_SUB:
        if(ltp->ty==PTR && rtp->ty==PTR)
            error("ポインタ型同士のひき算は定義されていません");
        if(ltp==type_int && rtp->ty==PTR)
            tp=rtp;
        if(ltp->ty==PTR && rtp==type_int)
            tp=ltp;
        break;
    case ND_MUL:
        if(ltp!=type_int || rtp!=type_int)
            error("数値型同士以外の掛け算は定義されていません");
        break;
    case ND_DIV:
        if(ltp!=type_int || rtp!=type_int)
            error("数値型同士以外の割り算は定義されていません");
        break;
    case ND_GRT:
        if(!equal(ltp,rtp))
            error("型が違うものを比較することはできません");
        break;
    case ND_GOE:
        if(!equal(ltp,rtp))
            error("型が違うものを比較することはできません");
        break;
    case ND_EQU:
        if(!equal(ltp,rtp))
            error("型が違うもの同士を比較することはできません");
        break;
    case ND_NEQ:
        if(!equal(ltp,rtp))
            error("型が違うもの同士を比較することはできません");
        break;
    default:
        error("未定義のタグが使用されています。");
        break;
    }

    node->type=tp;
    return tp;
}

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