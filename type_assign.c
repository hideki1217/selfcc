#include "selfcc.h"
#include<stdlib.h>

Type *type_assign(Node *node) {
    Type *tp;
    switch (node->kind) {
        case ND_NULL:
            tp = find_type_from_name("void");
            break;
        case ND_ROOTINE: {
            RootineNode *rootine = (RootineNode *)node;
            for (Node *nd = (Node *)rootine->arg; nd; nd = nd->next) {
                type_assign(nd);
            }
            Type *return_type = type_assign(rootine->block);
            // TODO:
            // 返り値について明示的なreturnがなければvoidでもいけることを実装
            // if(!equal(node->type->ptr_to,return_type))
            //    error("宣言での返り値の型と実際の型が一致しません。");
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
            if (rtp->kind == TY_FUNCTION) rtp = new_Pointer(rtp);
            if (!isLeftsidevalue(ltp))
                error_at(node->pos->str,
                         "変更可能な左辺値でなければいけません");
            if (!isAssignable(ltp, rtp))
                error_at(node->pos->str,
                         "右辺値の型が左辺値に代入できない型です。");
            tp = ltp;
            break;
        }
        case ND_RETURN: {
            tp = type_assign(((UnaryNode *)node)->target);
            break;
        }
        case ND_IF: {
            // TODO: 条件式がNULLの場合はfalseにする
            tp = type_assign(((CondNode *)node)->cond);

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
        case ND_CALL: {
            CallNode *fnode = ((CallNode *)node);
            Node *fptr = fnode->func;  // 呼び出す関数

            Type *ftype = type_assign(fptr);
            Callability call = isCallable(ftype);
            if (!call)
                error_at(fptr->pos->str,
                         "関数のように呼び出すことはできません。");

            Params act_par;
            set_Params(&act_par);
            Param par[ARG_MAX];
            int i = 0;
            // fnodeのargは逆順で出てくる
            for (Node *elem = fnode->arg; elem; elem = elem->next) {
                Param *param = par + (i++);
                set_Param(param, type_assign(elem));
                param->next = act_par.root;
                act_par.root = param;  // 逆順にいれていく
            }

            // 関数の引数チェック
            Params *exact =
                (call == AsFUNCTION) ? ftype->params : ftype->ptr_to->params;
            int res = params_compare(exact, &act_par);
            switch (res) {
                case 1:
                    error_at(node->pos->str, "引数の型が違います。");
                case 2:
                    error_at(node->pos->str, "引数が少なすぎます。");
                case 3:
                    error_at(node->pos->str, "引数が多すぎます。");
            }

            tp = ftype->ptr_to;  // 関数の返り値の型
            break;
        }
        case ND_ADDR: {
            tp = type_assign(((UnaryNode *)node)->target);
            tp = new_Pointer(tp);
            break;
        }
        case ND_DEREF: {
            tp = type_assign(((UnaryNode *)node)->target);
            if (!isArrayorPtr(tp))
                error_at(node->pos->str,
                         "ポインタ型Likeでない変数を参照できません。");
            tp = tp->ptr_to;
            break;
        }
        case ND_SET: {
            BlockNode *bnode = (BlockNode *)node;
            for (Node *elem = bnode->block; elem; elem = elem->next) {
                tp = type_assign(elem);
                if (elem->kind == ND_RETURN) break;
            }

            break;
        }
        case ND_LVARINIT:
        case ND_GVARINIT: {
            VarInitNode *inode = (VarInitNode *)node;
            if (inode->value) {
                if (node->kind == ND_SET) {
                    // TODO: 中括弧を使った初期化の型チェック
                } else {
                    Type *rtp = type_assign(inode->value);
                    if (!isAssignable(inode->var->type, rtp))
                        error_at(inode->var->name,
                                 "不正な初期化です。型が違います。");
                }
            }
            tp = inode->var->type;
            break;
        }
        case ND_INCRE:
        case ND_DECRE: {
            tp = type_assign(((BinaryNode *)node)->lhs);
            if (!isLeftsidevalue(tp))
                error_at(node->pos->str,
                         "変更可能な左辺値でなければいけません");
            break;
        }
        case ND_ADDASS:
        case ND_SUBASS: {
            Type *ltp = type_assign(((BinaryNode *)node)->lhs);
            Type *rtp = type_assign(((BinaryNode *)node)->rhs);
            if (!isAddSubable(ltp, rtp))
                error_at(node->pos->str,
                         "%s と %s は足したり引いたりできません。",
                         type2str(ltp), type2str(rtp));
            if (!isLeftsidevalue(ltp))
                error_at(node->pos->str,
                         "変更可能な左辺値でなければいけません");
            tp = ltp;
            break;
        }
        case ND_MULASS:
        case ND_DIVASS: {
            Type *ltp = type_assign(((BinaryNode *)node)->lhs);
            Type *rtp = type_assign(((BinaryNode *)node)->rhs);
            if (!isMulDivable(ltp, rtp))
                error_at(node->pos->str,
                         "%s と %s はかけたり、割ったりできません。",
                         type2str(ltp), type2str(rtp));
            if (!isLeftsidevalue(ltp))
                error_at(node->pos->str,
                         "変更可能な左辺値でなければいけません");
            tp = ltp;
            break;
        }
        case ND_INT:
        case ND_CHAR:
        case ND_FLOAT:
        case ND_ENUM:
        case ND_STR: {
            tp = node->type;
            break;
        }
        case ND_NOT:{
            tp = type_assign(((UnaryNode*)node)->target);
            if(isNum(tp) && !isInteger(tp))
                error_at(node->pos->str,"浮動小数点を反転することはできません。");
        }
        default: {
            Type *ltp = type_assign(((BinaryNode *)node)->lhs);
            Type *rtp = type_assign(((BinaryNode *)node)->rhs);

            Type *type_int = find_type_from_name("int");
            tp = type_int;  // default の型は今のところ int
            switch (node->kind) {
                case ND_ADD:
                    if (isArrayorPtr(ltp) && isArrayorPtr(rtp))
                        error_at(node->pos->str,
                                 "ポインタ型同士の足し算は定義されていません");
                    if (equal(ltp, type_int) && isArrayorPtr(rtp)) tp = rtp;
                    if (isArrayorPtr(ltp) && equal(rtp, type_int)) tp = ltp;
                    break;
                case ND_SUB:
                    if (isArrayorPtr(ltp) && isArrayorPtr(rtp)) tp = type_int;
                    if (equal(ltp, type_int) && isArrayorPtr(rtp)) tp = rtp;
                    if (isArrayorPtr(ltp) && equal(rtp, type_int)) tp = ltp;
                    break;
                case ND_MUL:
                    if (!isNum(ltp) || !isNum(rtp))
                        error_at(node->pos->str,
                                 "数値型同士以外の掛け算は定義されていません");
                    break;
                case ND_DIV:
                    if (!isNum(ltp) || !isNum(rtp))
                        error_at(node->pos->str,
                                 "数値型同士以外の割り算は定義されていません");
                    break;
                case ND_GRT:
                    if (!equal(ltp, rtp))
                        error_at(node->pos->str,
                                 "型が違うものを比較することはできません");
                    break;
                case ND_GOE:
                    if (!equal(ltp, rtp))
                        error_at(node->pos->str,
                                 "型が違うものを比較することはできません");
                    break;
                case ND_EQU:
                    if (!equal(ltp, rtp))
                        error_at(node->pos->str,
                                 "型が違うもの同士を比較することはできません");
                    break;
                case ND_NEQ:
                    if (!equal(ltp, rtp))
                        error_at(node->pos->str,
                                 "型が違うもの同士を比較することはできません");
                    break;
                case ND_RSHFT:
                case ND_LSHFT:
                    if(!isInteger(ltp) || !isInteger(rtp))
                        error_at(node->pos->str,"整数型でない型をシフトすることはできません。");
                    break;
                case ND_OR:
                case ND_XOR:
                case ND_AND:
                    if(!isInteger(ltp) || !isInteger(rtp))
                        error_at(node->pos->str,"式には整数型が必要です。");
                    break;
                case ND_MOD:
                    if(!isInteger(ltp) || !isInteger(rtp))
                        error_at(node->pos->str,"式には整数型が必要です。");
                    break;
                case ND_LGCOR:
                case ND_LGCAND:
                case ND_LGCNOT:
                case ND_GOTO:
                case ND_BREAK:
                case ND_CONTINUE:
                case ND_LABEL:
                case ND_CASE:
                case ND_DEFAULT:
                case ND_DOWHILE:
                case ND_SWITCH:
                case ND_CAST:
                case ND_MODASS:
                case ND_LSHASS:
                case ND_RSHASS:
                case ND_ANDASS:
                case ND_ORASS:
                case ND_XORASS:
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