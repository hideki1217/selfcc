#include "selfcc.h"

#include <stdio.h>

CVar *_node_evaluate_val(Node *node) {
    if (node->kind == ND_GVAR) {
        Var *var = ((VarNode *)node)->var;
        CVar *cvar = cc_avltree_Search(constant_tree,var->name,var->len);
        return cvar;
    }
    if (node->kind == ND_ACCESS) {// TODO: 未完成
        int total_offset = 0;
        Node *top = node;
        while (top->kind == ND_ACCESS) {
            total_offset += ((OffsetNode *)top)->tag.offset;
            top = ((OffsetNode *)top)->origin;
        }
        CVar *cvar = _node_evaluate_val(top);
        return NULL;
    }
    error_at(node->pos->str,
             "定数値が必要です。");
}
Node *_node_evaluate(Node *node) {// TODO: 現状intにだけ対応
    switch (node->kind) {
        case ND_INT: return node;
        case ND_STR: return node;
        case ND_LVAR:
        case ND_GVAR:
        case ND_ACCESS: //TODO: 構造体の定数は未対応
        {
            VarNode *vnode = (VarNode*)node;
            CVar *cvar = _node_evaluate_val(node);
            if(! isInteger(cvar->base.type))
                error(cvar->base.name,"現状int以外の定数値は扱えません");
            return (Node*)new_NumNode(cvar_int(cvar)); // だいぶ雑
        }
    }

    Node *lhs = ((BinaryNode *)node)->lhs;
    Node *rhs = ((BinaryNode *)node)->rhs;

    Type *ltp = lhs->type;
    Type *rtp = rhs->type;

    Node * lnode = _node_evaluate(lhs);
    Node * rnode = _node_evaluate(rhs);

    switch (node->kind) {
        case ND_ADD:
            if (isInteger(ltp) && isArrayorPtr(rtp)) {

            }
            if (isArrayorPtr(ltp) && isInteger(rtp)) {
                printf("    imul rdi, %d\n", type_size(type_ptr_to(lhs->type)));
            }
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            if (isInteger(ltp) && isArrayorPtr(rtp)) {
                printf("    imul rax, %d\n", type_size(type_ptr_to(rhs->type)));
            }
            if (isArrayorPtr(ltp) && isInteger(rtp)) {
                printf("    imul rdi, %d\n", type_size(type_ptr_to(lhs->type)));
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
            printf("    sar %s, cl\n", rax(ltp));
            break;
        case ND_LSHFT:
            printf("    mov ecx, edi\n");
            printf("    sal %s, cl\n", rax(ltp));
            break;
        case ND_OR:
            uniformSizedOper("or", ltp, rax, rtp, rdi);
            // printf("    or %s, %s",rax(ltp),rdi(rtp));
            break;
        case ND_XOR:
            uniformSizedOper("xor", ltp, rax, rtp, rdi);
            // printf("    xor %s, %s",rax(ltp),rdi(rtp));
            break;
        case ND_AND:
            uniformSizedOper("and", ltp, rax, rtp, rdi);
            // printf("    and %s, %s",rax(ltp),rdi(rtp));
            break;
        case ND_MOD:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            printf("    %s, %s\n", movsx2rax(ltp), rdx(ltp));
            break;
    }
}

Node *node_evaluate(Node *node) {
    type_assign(node);
    return _node_evaluate(node);
}
Node *node_getval_integer(Node *node) {
    Node *res = node_evaluate(node);
    if(! isAssignable(typemgr_find("int",3,BK_OTHER), res->type))
        error_at(node->pos->str,"整数型にキャストできる型が必要です。"); 
    return res;
}

Node *node_getnumval(Node *node) {
    Node *res = node_evaluate(node);
    return res;
}