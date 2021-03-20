#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "selfcc.h"
#include "utility.h"

Node *code;

void uniformSizedOper(char *oper,Type *ltp,char* (*lReg)(Type*),Type *rtp,char* (*rReg)(Type*));
void genArgs(Node *arg_top);

void gen_lval(Node *node, bool push) {
    if (node->kind == ND_LVAR) {
        LVar *var = (LVar *)((VarNode *)node)->var;
        printf("    lea rax, -%d[rbp]\n", var->offset);
        if (push) printf("    push rax\n");
        return;
    }
    if (node->kind == ND_GVAR) {
        Var *var = ((VarNode *)node)->var;
        string_ncopy(buffer, var->name, var->len);
        printf("    lea rax, %s[rip]\n", buffer);
        if (push) printf("    push rax\n");
        return;
    }
    if (node->kind == ND_DEREF) {
        gen(((UnaryNode *)node)->target, push);
        return;
    }
    if (isArrayorPtr(node->type)) {
        gen(node, push);
        return;
    }
    error_at(node->pos->str,
             "左辺値になれない値に代入や参照をしようとしました。");
}

void gen(Node *node, bool push) {
    int lcount;
    switch (node->kind) {
        case ND_NULL:
            return;
        case ND_ROOTINE: {
            RootineNode *rootine = (RootineNode *)node;
            string_ncopy(buffer, rootine->func->name, rootine->func->len);
            printf("%s:\n", buffer);
            // プロローグ
            // local変数分の領域を確保　
            printf("    push rbp\n");
            printf("    mov rbp, rsp\n");
            printf("    sub rsp, %d\n", rootine->total_offset);

            //引数の読み込み(ローカル変数として)
            int count = 0;
            Type *tp;
            for (VarNode *var = rootine->arg; var;
                 var = (VarNode *)var->base.next, count++) {
                gen_lval((Node *)var, false);
                tp = ((Node *)var)->type;
                if (count < 6)
                    printf("    mov [rax], %s\n", registry_for_arg(tp, count));
                else {
                    printf("    mov %s, %s [rbp+%d]\n", rdi(tp), sizeoption(tp),
                           8 * (count - 6) + 16);
                    printf("    mov [rax], %s\n", rdi(tp));
                }
            }

            gen(rootine->block, false);

            // エピローグ
            // 最後の式の結果がRAXに残っているのでそれが返り値になる
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");

            return;
        }
        case ND_GVARINIT: {
            VarInitNode *inode = (VarInitNode *)node;
            Var *var = inode->var;
            if (inode->value) {
                // グローバル変数の初期化についてはここに書くべし
            } else {
                string_ncopy(buffer, var->name, var->len);
                printf("    .comm %s,%d\n", buffer, var->type->size);
            }
            return;
        }
        /////////////////////////////// 以下push flagが有効
        case ND_INT: {
            printf("    mov rax, %d\n", ((NumNode *)node)->val);
            if (push) printf("    push rax\n");
            return;
        }
        case ND_STR: {
            printf("    lea rax, .LC%d[rip]\n", ((ConstNode *)node)->const_id);
            if (push) printf("    push rax\n");
            return;
        }
        case ND_LVAR:
        case ND_GVAR: {
            VarNode *vnode = (VarNode *)node;
            gen_lval(node, push);
            if (vnode->var->type->kind == TY_ARRAY ||
                vnode->var->type->kind == TY_FUNCTION)
                return;  // 配列型と関数型はPOINTERみたいにしてあげる
            if (push) printf("    pop rax\n");
            printf("    %s, %s [rax]\n",
                   movsx2rax(node->type),  //符号拡張ありになっている
                   sizeoption(node->type));
            if (push) printf("    push rax\n");
            return;
        }
        case ND_ASSIGN: {
            BinaryNode *bi = (BinaryNode *)node;
            gen_lval(bi->lhs, true);
            gen(bi->rhs, false);

            printf("    pop rdi\n");
            printf("    mov [rdi], %s\n", rax(bi->lhs->type));
            if (push) printf("    push rax\n");

            return;
        }
        case ND_RETURN: {
            UnaryNode *un = (UnaryNode *)node;
            gen(un->target, false);
            //エピローグ
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return;
        }
        case ND_IF: {
            CondNode *cnode = (CondNode *)node;
            lcount = cnode->index;
            gen(cnode->cond, false);
            printf("    cmp rax, 0\n");
            if (push) printf("    push rax\n");
            printf("    je .Lend%d\n", lcount);
            gen(cnode->T, push);
            printf(".Lend%d:\n", lcount);
            return;
        }
        case ND_IFEL: {
            CondNode *cnode = (CondNode *)node;
            lcount = cnode->index;
            gen(cnode->cond, false);
            printf("    cmp rax, 0\n");
            printf("    je .Lelse%d\n", lcount);
            gen(cnode->T, push);
            printf("    jmp .Lend%d\n", lcount);
            printf(".Lelse%d:\n", lcount);
            gen(cnode->F, push);
            printf(".Lend%d:\n", lcount);
            return;
        }
        case ND_WHILE: {
            CondNode *cnode = (CondNode *)node;
            lcount = cnode->index;
            printf(".Lbegin%d:\n", lcount);
            gen(cnode->cond, false);
            printf("    cmp rax, 0\n");
            printf("    je .Lend%d\n", lcount);
            gen(cnode->T, false);
            printf("    jmp .Lbegin%d\n", lcount);
            printf(".Lend%d:\n", lcount);
            if (push) printf("    push rax\n");
            return;
        }
        case ND_DOWHILE:{
            CondNode *cnode = (CondNode *)node;
            lcount = cnode->index;
            printf(".Lbegin%d:\n", lcount);
            gen(cnode->T, false);
            gen(cnode->cond, false);
            printf("    cmp rax, 0\n");
            printf("    je .Lend%d\n", lcount);
            printf("    jmp .Lbegin%d\n", lcount);
            printf(".Lend%d:\n", lcount);
            if (push) printf("    push rax\n");
            return;
        }
        case ND_FOR: {
            ForNode *fnode = (ForNode *)node;
            lcount = fnode->index;
            gen(fnode->init, false);
            printf(".Ltop%d:\n", lcount);
            gen(fnode->cond, false);
            printf("    cmp rax, 0\n");
            printf("    je .Lend%d\n", lcount);
            gen(fnode->T, false);
            printf(".Lbegin%d:\n",lcount);
            gen(fnode->update, false);
            printf("    jmp .Ltop%d\n", lcount);
            printf(".Lend%d:\n", lcount);
            if (push) printf("    push rax\n");
            return;
        }
        case ND_BLOCK: {
            gen(((BlockNode *)node)->block, push);
            return;
        }
        case ND_CALL: {
            CallNode *fnode = ((CallNode *)node);
            Node *function = fnode->func;
            int floatCount = 0;

            // 引数をセット
            genArgs(fnode->arg);

            // 可変長引数の関数を呼ぶ際は、浮動小数点数の引数の個数をalに入れる
            printf("    mov rax, %d\n",floatCount);

            Callability call = isCallable(function->type);
            if (function->kind == ND_GVAR && call == AsFUNCTION) {
                VarNode *vfunc = (VarNode *)function;
                string_ncopy(buffer, vfunc->var->name, vfunc->var->len);
                printf("    call %s\n", buffer);
            } else {
                if (call == AsFUNCTION) {
                    gen(function, false);
                } else if (call == AsPTR2FUNC) {
                    UnaryNode tmp;
                    set_UnaryNode(&tmp, ND_DEREF, function);
                    tmp.base.type = function->type->ptr_to;
                    gen((Node *)&tmp, false);
                }
                printf("    call rax\n");
            }
            if (push) printf("    push rax\n");
            return;
        }
        case ND_ADDR: {
            gen_lval(((UnaryNode *)node)->target, push);
            return;
        }
        case ND_DEREF: {
            UnaryNode *unode = (UnaryNode *)node;
            gen(unode->target, false);
            if (node->type->kind != TY_FUNCTION) {
                printf("    %s, %s [rax]\n",
                       movsx2rax(unode->target->type
                                     ->ptr_to),  // 符号拡張ありになっている
                       sizeoption(unode->target->type->ptr_to));
            }
            if (push) printf("    push rax\n");
            return;
        }
        case ND_SET: {
            BlockNode *bnode = (BlockNode *)node;
            for (Node *elem = bnode->block; elem; elem = elem->next) {
                gen(elem, false);
                if (elem->kind == ND_RETURN) break;
            }
            if (push) printf("    push rax\n");

            return;
        }
        case ND_LVARINIT: {
            VarInitNode *inode = (VarInitNode *)node;
            if (inode->value) {
                LVar *var = (LVar *)inode->var;
                if (var->base.type->kind == TY_ARRAY) {
                    if (inode->value) {
                        // TODO: 配列の宣言時初期化について書くならここ
                    }
                } else if (var->base.type->kind == TY_STRUCT) {
                    if (inode->value) {
                        // TODO: structの宣言時初期化について書くならここ
                    }
                } else {
                    // 仮の代入文を生成
                    VarNode vnode;
                    set_VarNode(&vnode, inode->var);
                    BinaryNode bnode;
                    set_BinaryNode(&bnode, ND_ASSIGN, (Node *)&vnode,
                                   inode->value);

                    gen((Node *)&bnode, false);
                }
            }
            if (push) printf("    push rax\n");
            return;
        }
        case ND_INCRE: {
            BinaryNode *bnode = (BinaryNode *)node;

            gen_lval(bnode->lhs, false);
            printf("    push [rax]\n");
            printf("    push rax\n");
            BinaryNode tmp;
            set_BinaryNode(&tmp, ND_ADD, bnode->lhs, bnode->rhs);
            type_assign((Node *)&tmp);
            gen((Node *)&tmp, false);
            printf("    pop rdi\n");
            printf("    mov [rdi], %s\n", rax(node->type));
            printf("    pop rax\n");
            if (push) printf("   push rax\n");
            return;
        }
        case ND_DECRE: {
            BinaryNode *bnode = (BinaryNode *)node;

            gen_lval(bnode->lhs, false);
            printf("    push [rax]\n");
            printf("    push rax\n");
            BinaryNode tmp;
            set_BinaryNode(&tmp, ND_SUB, bnode->lhs, bnode->rhs);
            type_assign((Node *)&tmp);
            gen((Node *)&tmp, false);
            printf("    pop rdi\n");
            printf("    mov [rdi], %s\n", rax(node->type));
            printf("    pop rax\n");
            if (push) printf("   push rax\n");
            return;
        }
        case ND_ADDASS: 
        case ND_SUBASS: 
        case ND_MULASS:
        case ND_DIVASS: 
        case ND_RSHASS:
        case ND_LSHASS:
        case ND_ORASS:
        case ND_ANDASS:
        case ND_XORASS:
        case ND_MODASS:{ 
            BinaryNode *bnode = (BinaryNode *)node,tmp;
            set_BinaryNode(&tmp, pairOf(node->kind), bnode->lhs, bnode->rhs);
            type_assign((Node *)&tmp);

            gen_lval(bnode->lhs, true);
            gen((Node *)&tmp, false);
            printf("    pop rdi\n");
            printf("    mov [rdi], %s\n", rax(node->type));
            if (push) printf("   push rax\n");
            return;
        }
        case ND_BREAK:{
            LabelNode *lnode = (LabelNode*)node;
            printf("    jmp .Lend%d\n",lnode->jumpTo);
            if (push) printf("   push rax\n");
            return;
        }
        case ND_CONTINUE:{
            LabelNode *lnode = (LabelNode*)node;
            printf("    jmp .Lbegin%d\n",lnode->jumpTo);
            if (push) printf("   push rax\n");
            return;
        }
        case ND_NOT:{
            UnaryNode *unode = (UnaryNode*)node;
            gen(unode->target,false);
            printf("    not rax\n");
            if (push) printf("   push rax\n");
            return;
        }
    }

    Node *lhs = ((BinaryNode *)node)->lhs;
    Node *rhs = ((BinaryNode *)node)->rhs;

    Type *ltp = lhs->type;
    Type *rtp = rhs->type;

    gen(lhs, true);
    gen(rhs, true);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    Type *type_int = find_type_from_name("int");

    switch (node->kind) {
        case ND_ADD:
            if (equal(ltp, type_int) && isArrayorPtr(rtp)) {
                printf("    imul rax, %d\n", rhs->type->ptr_to->size);
            }
            if (isArrayorPtr(ltp) && equal(rtp, type_int)) {
                printf("    imul rdi, %d\n", lhs->type->ptr_to->size);
            }
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            if (equal(ltp, type_int) && isArrayorPtr(rtp)) {
                printf("    imul rax, %d\n", rhs->type->ptr_to->size);
            }
            if (isArrayorPtr(ltp) && equal(rtp, type_int)) {
                printf("    imul rdi, %d\n", lhs->type->ptr_to->size);
            }
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
        case ND_RSHFT:
            printf("    mov ecx, edi\n"); 
            printf("    sar %s, cl\n",rax(ltp));
            break;
        case ND_LSHFT:
            printf("    mov ecx, edi\n"); 
            printf("    sal %s, cl\n",rax(ltp));
            break;
        case ND_OR:
            uniformSizedOper("or",ltp,rax,rtp,rdi);
            //printf("    or %s, %s",rax(ltp),rdi(rtp));
            break;
        case ND_XOR:
            uniformSizedOper("xor",ltp,rax,rtp,rdi);
            //printf("    xor %s, %s",rax(ltp),rdi(rtp));
            break;
        case ND_AND:
            uniformSizedOper("and",ltp,rax,rtp,rdi);
            //printf("    and %s, %s",rax(ltp),rdi(rtp));
            break;
        case ND_MOD:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            printf("    %s, %s\n",movsx2rax(ltp),rdx(ltp));
            break;
        
        case ND_LGCOR:
        case ND_LGCAND:
        case ND_LGCNOT:
        case ND_GOTO:
        
        case ND_LABEL:
        case ND_CASE:
        case ND_DEFAULT:
        
        case ND_SWITCH:
        case ND_CAST:
        
        case ND_ENUM:
        case ND_FLOAT:
        case ND_CHAR:
        default:
            error("未定義のタグが使用されています。");
            break;
    }

    if (push) printf("    push rax\n");
    return;
}

void uniformSizedOper(char *oper,Type *ltp,char* (*lReg)(Type* type),Type *rtp,char* (*rReg)(Type* type)){
    if(ltp->size==rtp->size)
        printf("    %s %s, %s\n",oper,lReg(ltp),rReg(rtp));
    else if(ltp->size > rtp->size){
        printf("    movsx %s, %s\n",rReg(ltp),rReg(rtp));
        printf("    %s %s, %s\n",oper,lReg(ltp),rReg(ltp));
    }
    else{
        printf("    movsx %s, %s\n",lReg(rtp),lReg(ltp));
        printf("    %s %s, %s\n",oper,lReg(rtp),rReg(rtp));
    }
}
void genArgs(Node *arg_top){
    int argcount = 0;
    for (Node *elem = arg_top; elem; elem = elem->next) argcount++;
    Type *mem[argcount];
    int i = argcount - 1;
    for (Node *elem = arg_top; elem; elem = elem->next, i--) {
        gen(elem, true);
        if (i < 6) mem[i] = elem->type;
    }
    for (int i = 0; i < min(6, argcount); i++) {
        printf("    pop rax\n");
        printf("    mov %s, %s\n", registry_for_arg(mem[i], i),
                rax(mem[i]));
    }
}