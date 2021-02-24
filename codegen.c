#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "selfcc.h"
#include "utility.h"

Type *type_assign(Node *node) {
    Type *tp;
    switch (node->kind) {
        case ND_ROOTINE: {
            RootineNode *rootine = (RootineNode *)node;

            //引数の読み込み(ローカル変数として)
            int count = 0;
            for (VarNode *var = rootine->arg; var;
                 var = (VarNode *)var->base.next, count++) {
                Type *vtp = type_assign(
                    (Node *)
                        var);  // TODO: 関数の情報から引数の型があっているか確認
            }

            tp = type_assign(
                rootine->block);  // TODO:
                                  // 関数の情報から返り値の型があっているか確認

            break;
        }
        case ND_NUM: {
            tp = ((NumNode *)node)->type;
            break;
        }
        case ND_STR: {
            tp =((ConstNode*)node)->base.type;
            break;
        }
        case ND_LVAR:
        case ND_GVAR: {
            tp = ((VarNode *)node)->var->type;
            break;
        }
        case ND_ASSIGN: {
            Type *ltp = type_assign(((BinaryNode *)node)->lhs);
            Type *rtp = type_assign(((BinaryNode *)node)->rhs);

            if (!isAssignable(ltp, rtp))
                error("右辺値の型が左辺値に代入できない型です。");
            tp = ltp;
            break;
        }
        case ND_RETURN: {
            tp = type_assign(((BinaryNode *)node)->lhs);
            break;
        }
        case ND_IF: {
            tp = type_assign(
                ((CondNode *)node)
                    ->cond);  // TODO: 条件式がNULLの場合はfalseにする

            tp = type_assign(((CondNode *)node)->T);

            break;
        }
        case ND_IFEL: {
            tp = type_assign(((CondNode *)node)->cond);

            Type *ttp = type_assign(((CondNode *)node)->T);
            Type *ftp = type_assign(((CondNode *)node)->F);

            tp = ttp;  // TODO: ここの型管理は未完成
            break;
        }
        case ND_WHILE: {
            tp = type_assign(((CondNode *)node)->cond);

            tp = type_assign(((CondNode *)node)->T);

            break;
        }
        case ND_FOR: {
            tp = type_assign(((ForNode *)node)->init);

            tp = type_assign(((ForNode *)node)->cond);

            tp = type_assign(((ForNode *)node)->T);

            type_assign(((ForNode *)node)->update);

            break;
        }
        case ND_BLOCK: {
            Type *ret_type = NULL;
            tp = type_assign(((BlockNode *)node)->block);

            break;
        }
        case ND_FUNCTION: {
            FuncNode *fnode = ((FuncNode *)node);

            for (Node *elem = fnode->arg; elem; elem = elem->next) {
                tp = type_assign(elem);  // TODO:関数の引数チェック
            }
            tp = find_type_from_name("int");  // TODO: 関数の型を返す
            break;
        }
        case ND_ADDR: {
            tp = type_assign(((BinaryNode *)node)->lhs);
            tp = new_Pointer(tp);
            break;
        }
        case ND_DEREF: {
            tp = type_assign(((BinaryNode *)node)->lhs);
            if (!isArrayorPtr(tp))
                error("ポインタ型Likeでない変数を参照できません。");
            tp = tp->ptr_to;
            break;
        }
        case ND_SET: {
            for (Node *elem = node->next; elem; elem = elem->next) {
                tp = type_assign(elem);
                if (elem->kind == ND_RETURN) break;
            }

            break;
        }
        case ND_LVARINIT:
        case ND_GVARINIT: {
            VarInitNode *inode = (VarInitNode *)node;
            tp = inode->var->type;
            break;
        }
        default: {
            Type *ltp = type_assign(((BinaryNode *)node)->lhs);
            Type *rtp = type_assign(((BinaryNode *)node)->rhs);

            Type *type_int = find_type_from_name("int");
            tp = type_int;
            switch (node->kind) {
                case ND_ADD:
                    if (isArrayorPtr(ltp) && isArrayorPtr(rtp))
                        error("ポインタ型同士の足し算は定義されていません");
                    if (ltp == type_int && isArrayorPtr(rtp)) tp = rtp;
                    if (isArrayorPtr(ltp) && rtp == type_int) tp = ltp;
                    break;
                case ND_SUB:
                    if (isArrayorPtr(ltp) && isArrayorPtr(rtp)) tp = type_int;
                    if (ltp == type_int && isArrayorPtr(rtp)) tp = rtp;
                    if (isArrayorPtr(ltp) && rtp == type_int) tp = ltp;
                    break;
                case ND_MUL:
                    if (ltp != type_int || rtp != type_int)
                        error("数値型同士以外の掛け算は定義されていません");
                    break;
                case ND_DIV:
                    if (ltp != type_int || rtp != type_int)
                        error("数値型同士以外の割り算は定義されていません");
                    break;
                case ND_GRT:
                    if (!equal(ltp, rtp))
                        error("型が違うものを比較することはできません");
                    break;
                case ND_GOE:
                    if (!equal(ltp, rtp))
                        error("型が違うものを比較することはできません");
                    break;
                case ND_EQU:
                    if (!equal(ltp, rtp))
                        error("型が違うもの同士を比較することはできません");
                    break;
                case ND_NEQ:
                    if (!equal(ltp, rtp))
                        error("型が違うもの同士を比較することはできません");
                    break;
                default:
                    error("未定義のタグが使用されています。");
                    break;
            }
            break;
        }
    }

    node->type = tp;
    return tp;
}

void gen_lval(Node *node) {
    if (node->kind == ND_LVAR) {
        LVar *var = (LVar *)((VarNode *)node)->var;
        printf("    lea rax, -%d[rbp]\n", var->offset);
        printf("    push rax\n");
        return;
    }
    if (node->kind == ND_GVAR) {
        Var *var = ((VarNode *)node)->var;
        char s[var->len + 1];
        string_limitedcopy(s, var->name, var->len);
        printf("    lea rax, %s[rip]\n", s);
        printf("    push rax\n");
        return;
    }
    if (node->kind == ND_DEREF) {
        gen(((BinaryNode *)node)->lhs);
        return;
    }
    gen(node);
    return;
}

void gen(Node *node) {
    int lcount;
    switch (node->kind) {
        case ND_ROOTINE: {
            RootineNode *rootine = (RootineNode *)node;
            char name[rootine->namelen + 1];
            string_limitedcopy(name, rootine->name, rootine->namelen);
            printf("%s:\n", name);
            // プロローグ
            // local変数分の領域を確保　
            printf("    push rbp\n");
            printf("    mov rbp, rsp\n");
            printf("    sub rsp, %d\n", rootine->total_offset);

            //引数の読み込み(ローカル変数として)
            int count = 0;
            for (VarNode *var = rootine->arg; var;
                 var = (VarNode *)var->base.next, count++) {
                gen_lval((Node *)var);
                printf("    pop rax\n");
                if (count < 6)
                    printf("    mov [rax], %s\n",
                           registry_for_arg(((Node *)var)->type, count));
                else
                    printf("    mov %s [rax], [rbp+%d]\n",
                           sizeoption(((Node *)var)->type),
                           (count - 6) * 8 + 16);  //ここ微妙
            }

            gen(rootine->block);

            // エピローグ
            // 最後の式の結果がRAXに残っているのでそれが返り値になる
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");

            return;
        }
        case ND_NUM: {
            printf("    push %d\n", ((NumNode *)node)->val);
            return;
        }
        case ND_STR: {
            printf("    lea rax, .LC%d[rip]\n",((ConstNode*)node)->const_id);
            printf("    push rax\n");
            return;
        }
        case ND_LVAR:
        case ND_GVAR: {
            gen_lval(node);
            if (((VarNode *)node)->var->type->kind == TY_ARRAY)
                return;  // 配列型はPOINTERみたいにしてあげる
            printf("    pop rax\n");
            printf("    %s, %s [rax]\n",
                    movzx2rax(node->type), //符号拡張なしになっている 
                    sizeoption(node->type));
            printf("    push rax\n");
            return;
        }
        case ND_ASSIGN: {
            BinaryNode *bi = (BinaryNode *)node;
            gen_lval(bi->lhs);
            gen(bi->rhs);

            printf("    pop rdi\n");
            printf("    pop rax\n");
            printf("    mov [rax], %s\n",
                    rdi(bi->rhs->type));
            printf("    push rdi\n");

            return;
        }
        case ND_RETURN: {
            BinaryNode *bi = (BinaryNode *)node;
            gen(bi->lhs);
            //エピローグ
            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return;
        }
        case ND_IF: {
            lcount = Lcount++;
            gen(((CondNode *)node)->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    push rax\n");
            printf("    je .Lend%d\n", lcount);
            gen(((CondNode *)node)->T);
            printf(".Lend%d:\n", lcount);
            return;
        }
        case ND_IFEL: {
            lcount = Lcount++;
            gen(((CondNode *)node)->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    push rax\n");
            printf("    je .Lelse%d\n", lcount);
            gen(((CondNode *)node)->T);
            printf("    jmp .Lend%d\n", lcount);
            printf(".Lelse%d:\n", lcount);
            gen(((CondNode *)node)->F);
            printf(".Lend%d:\n", lcount);
            return;
        }
        case ND_WHILE: {
            lcount = Lcount++;
            printf(".Lbegin%d:\n", lcount);
            gen(((CondNode *)node)->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    push rax\n");
            printf("    je .Lend%d\n", lcount);
            gen(((CondNode *)node)->T);
            printf("    jmp .Lbegin%d\n", lcount);
            printf(".Lend%d:\n", lcount);
            return;
        }
        case ND_FOR: {
            lcount = Lcount++;
            gen(((ForNode *)node)->init);
            printf(".Lbegin%d:\n", lcount);
            gen(((ForNode *)node)->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    push rax\n");
            printf("    je .Lend%d\n", lcount);
            gen(((ForNode *)node)->T);
            gen(((ForNode *)node)->update);
            printf("    jmp .Lbegin%d\n", lcount);
            printf(".Lend%d:\n", lcount);
            return;
        }
        case ND_BLOCK: {
            gen(((BlockNode *)node)->block);
            return;
        }
        case ND_FUNCTION: {
            FuncNode *fnode = ((FuncNode *)node);
            int argcount = 0;
            for (Node *elem = fnode->arg; elem; elem = elem->next, argcount++)
                ;
            Type *mem[argcount];
            int i = argcount - 1;
            for (Node *elem = fnode->arg; elem; elem = elem->next, i--) {
                gen(elem);
                if (i < 6) mem[i] = elem->type;
            }
            for (int i = 0; i < min(6, argcount); i++) {
                printf("    pop rax\n");
                printf("    mov %s, %s\n", 
                        registry_for_arg(mem[i], i),
                        rax(mem[i]));
            }

            char str[fnode->namelen + 1];
            string_limitedcopy(str, fnode->funcname, fnode->namelen);
            printf("    call %s\n", str);
            printf("    push rax\n");
            return;
        }
        case ND_ADDR: {
            gen_lval(((BinaryNode *)node)->lhs);
            return;
        }
        case ND_DEREF: {
            BinaryNode *bi = (BinaryNode *)node;
            gen(bi->lhs);
            printf("    pop rax\n");
            printf("    %s, %s [rax]\n",
                    movzx2rax(bi->lhs->type->ptr_to),// 符号拡張なしになっている
                    sizeoption(bi->lhs->type->ptr_to));
            printf("    push rax\n");
            return;
        }
        case ND_SET: {
            for (Node *elem = node->next; elem; elem = elem->next) {
                gen(elem);
                printf("    pop rax\n");
                if (elem->kind == ND_RETURN) break;
            }
            printf("    push rax\n");

            return;
        }
        case ND_LVARINIT: {
            VarInitNode *inode = (VarInitNode *)node;
            LVar *var = (LVar *)inode->var;
            if (var->base.type->kind == TY_ARRAY) {
                printf("    push rax\n");
                if (inode->value) {
                    // TODO: 配列の宣言時初期化について書くならここ
                }
            } else {
                error(
                    "現状普通の変数の場合は元の構文を使うよ。");  // TODO:　ここを消すかどうか考えるべし
            }
            return;
        }
        case ND_GVARINIT: {
            VarInitNode *inode = (VarInitNode *)node;
            Var *var = inode->var;
            if (inode->value) {
                // グローバル変数の初期化についてはここに書くべし
            } else {
                char s[var->len + 1];
                string_limitedcopy(s, var->name, var->len);
                printf("    .comm %s,%d\n", s, var->type->size);
            }
            return;
        }
    }

    Node *lhs = ((BinaryNode *)node)->lhs;
    Node *rhs = ((BinaryNode *)node)->rhs;

    Type *ltp = lhs->type;
    Type *rtp = rhs->type;

    gen(lhs);
    gen(rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    Type *type_int = find_type_from_name("int");

    switch (node->kind) {
        case ND_ADD:
            if (ltp == type_int && isArrayorPtr(rtp)) {
                printf("    imul rax, %d\n", rhs->type->ptr_to->size);
            }
            if (isArrayorPtr(ltp) && rtp == type_int) {
                printf("    imul rdi, %d\n", lhs->type->ptr_to->size);
            }
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            if (ltp == type_int && isArrayorPtr(rtp)) {
                printf("    imul rax, %d\n", rhs->type->ptr_to->size);
            }
            if (isArrayorPtr(ltp) && rtp == type_int) {
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
        default:
            break;
    }

    printf("    push rax\n");

    return;
}