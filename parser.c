#include <stdlib.h>

#include "selfcc.h"

#define ISCONST 1
#define ISVOLATILE 2

#define ISNULL(a) (a) == NULL
#define ISNNULL(a) (a) == NULL

//文法部
static int structId = 0;
static int unionId = 0;
static int enumId = 0;
int LCcount = 0;
Node *nullNode;

Type *declarator(Type *base, Token **ident) {
    *ident = NULL;
    Type *tp = base;
    while (consume("*")) {
        tp = new_Pointer(tp);
        tp->isConst |= ISNULL(consume("const"));
        tp->isVolatile |= ISNULL(consume("volatile"));
        // error_here(true, "型を修飾する語でなければなりません。");
    }
    *ident = direct_declarator(&tp);
    return tp;
}
Token *direct_declarator(Type **base) {
    Type type, *ptr = &type;
    type.ptr_to = NULL;
    Token *identName = NULL;
    if (consume("(")) {
        ptr = declarator(ptr, &identName);
        expect(')');
    } else
        identName = consume_ident();
    // if (tk == NULL) error_here(false, "宣言すべき識別子が存在しません。");
    if (consume("[")) {
        int size = expect_integer();  // TODO:　定数式ならおっけ
        *base = new_Array(*base, size);
        expect(']');
    }
    if (consume("(")) {
        Params *params = new_Params();
        Type *now_type;
        Token *name = NULL;
        while (!consume(")")) {
            consume(",");
            if (consume("...")) {
                params_addVaArg(params);
                expect(')');
                break;
            }
            // 引数の型名を読む
            StorageMode mode;
            if (!declaration_specifier(&mode, &now_type))
                error_here(true, "型名が存在しません");
            now_type = declarator(now_type, &name);

            params_addParam(params, now_type);
            params_setIdent(params, name);
        }
        *base = new_Function(*base, params);
    }
    Type *res = ptr;
    while (1) {
        if (res == &type) break;
        if (res->ptr_to == &type) {
            res->ptr_to = *base;
            *base = ptr;
            break;
        }
        res = res->ptr_to;
    }

    return identName;
}
bool specifier_qualifier(Type **tp, bool errorExpected) {
    Type *tmp = NULL;
    flag_n flag = 0;
    bool updated = true;
    int count = 0;
    while (updated) {
        if ((tmp = type_specifier()) != NULL) {
            if (tp != NULL)
                error_here(false, "宣言において型は一意である必要があります。");
            updated = true;
        }
        if ((flag |= type_qualifier()) != 0) {
            updated = true;
        }
        updated = false;
        count++;
    }
    if (!errorExpected & count <= 1) return false;
    if (tmp == NULL) error_here(false, "ベースの型が宣言されていません");
    *tp = tmp;
    (*tp)->isConst = flag & ISCONST;
    (*tp)->isVolatile = flag & ISVOLATILE;
    return count <= 1;
}
Type *type_name(bool isCheck) {
    Type *tp = NULL;
    if (isCheck & !specifier_qualifier(&tp, !isCheck)) return NULL;
    abstract_declarator(&tp);
    return tp;
}
void abstract_declarator(Type **base) {
    bool isConst = false;
    bool isVolatile = false;
    Type type, *ptr = &type;
    type.ptr_to = NULL;
    Token *tk = NULL;
    while (consume("*")) {
        *base = new_Pointer(*base);
        (*base)->isConst |= ISNNULL(consume("const"));
        (*base)->isVolatile |= ISNNULL(consume("volatile"));
    }
    if (consume("(")) {
        abstract_declarator(&ptr);
        expect(')');
    }
    if (tk == NULL) error_here(false, "ここに識別子は存在してはいけません");
    if (consume("[")) {
        int size = expect_integer();  // TODO:　定数式ならおっけ
        *base = new_Array(*base, size);
        expect(']');
    }
    if (consume("(")) {
        Params *params = new_Params();
        Type *now_type;
        Token *tk;
        while (!consume(")")) {
            consume(",");
            if (consume("...")) {
                params_addVaArg(params);
                expect(')');
                break;
            }
            now_type = type_name(false);
            tk = consume_ident();
            params_addParam(params, now_type);
        }
        *base = new_Function(*base, params);
        expect(')');
    }
    Type *res = ptr;
    while (1) {
        if (res == &type) break;
        if (res->ptr_to == &type) {
            res->ptr_to = *base;
            *base = ptr;
            break;
        }
        res = res->ptr_to;
    }
}

StorageMode storage_specifier() {
    if (consume("typedef")) return SM_TYPEDEF;
    if (consume("extern")) return SM_EXTERN;
    if (consume("static")) return SM_STATIC;
    if (consume("auto")) return SM_AUTO;
    if (consume("register")) return SM_REGISTER;
    return SM_NONE;
}
Type *type_specifier() {  // TODO: 未実装
    Type *tp = NULL;
    // struct宣言
    if (consume("struct")) {
        return tp;
    }
    // union宣言
    if (consume("union")) {
        return tp;
    }
    // enum宣言
    if (consume("enum")) {
        return tp;
    }
    // 符号付き
    if (consume("signed")) {
        if (consume("char")) {
        }
        if (consume("short")) {
        }
        if (consume("int")) {
        }
        if (consume("long")) {
        }
        return tp;
    }
    // 符号なし
    if (consume("unsigned")) {
        if (consume("char")) {
        }
        if (consume("short")) {
        }
        if (consume("int")) {
        }
        if (consume("long")) {
        }
        return tp;
    }
    if (consume_Type(&tp)) return tp;
    return NULL;
}
flag_n type_qualifier() {
    bool isConst = false, isVolatile = false;
    flag_n flag = 0;
    if (!isConst && consume("const")) isConst = true, flag += ISCONST;
    if (!isVolatile && consume("volatile"))
        isVolatile = true, flag += ISVOLATILE;
    return flag;
}
Node *local_declaration(bool asExpr) {
    StorageMode mode = SM_NONE;
    Type *tp = NULL;
    Token *ident;
    if (!declaration_specifier(&mode, &tp)) return NULL;
    tp = declarator(tp, &ident);

    Node *res;
    if (CanbeFuncDef(tp)) {
        switch (mode) {
            case SM_AUTO:
            case SM_REGISTER:
            case SM_TYPEDEF:
                error_at(ident->str, "関数に対して無効なストレージ クラスです");
            case SM_EXTERN:
            case SM_NONE: {
                ExVar *var = add_exvar(ident, tp);
                res = new_Node(ND_NULL);
                break;
            }
            case SM_STATIC:
                error_at(ident->str,
                         "blockscopeではextern修飾子のみが許可されています。");
        }
    } else {
        switch (mode) {
            case SM_TYPEDEF: {
                Type *alias = new_Alias(tp, ident->str, ident->len);
                regist_type(alias);
                res = new_Node(ND_NULL);
                break;
            }
            case SM_EXTERN: {
                ExVar *var = add_exvar(ident, tp);
                res = new_Node(ND_NULL);
                break;
            }
            case SM_AUTO:
            case SM_REGISTER:
            case SM_NONE: {
                LVar *var = add_lvar(ident, tp);
                Node *value = NULL;
                if (consume("=")) {  // 初期化
                    value = initilizer();
                }
                res = (Node*)new_VarInitNode((Var *)var, value);
                break;
            }
            case SM_STATIC: {
                GVar *var = add_gvar(ident, tp, true);
                Node *value = NULL;
                if (consume("=")) {  // 初期化
                    value = initilizer();
                }
                res = (Node*)new_VarInitNode((Var *)var, value);
                break;
            }
        }
    }
    if(!asExpr)expect(';');
    return res;
}
/**
 * @brief  宣言付きのParamsからVarNodeのlistを作成
 * @note   
 * @param  *params: 対象のParams
 * @retval VarNodeのlistの先頭(Paramsと同順)
 */
VarNode *CreateArgs(Params *params) {
    VarNode anker;
    anker.base.next = NULL;
    VarNode *top = &anker,*tmp;
    for (Param *par = params->root; par && par->kind != PA_VAARG; par = par->next) {
        if (par->token == NULL) error("引数の識別子が存在しません。");
        Var *var = (Var*)add_lvar(par->token,par->type);
        tmp = new_VarNode(var);
        top->base.next = (Node*)tmp;
        top = tmp; 
    }
    return (VarNode*)anker.base.next;
}
Node *global_declaration() {
    StorageMode mode = SM_NONE;
    Type *tp = NULL;
    Token *ident;

    if (!declaration_specifier(&mode, &tp)) return NULL;
    tp = declarator(tp, &ident);

    if (CanbeFuncDef(tp)) {
        lvar_manager_Clear(locals);
        switch (mode) {
            case SM_AUTO:
            case SM_REGISTER:
            case SM_TYPEDEF:
                error_at(ident->str,
                         "関数定義が不正な修飾子を宣言しています。");
            case SM_EXTERN: {
                expect(';');
                ExVar *var = add_exvar(ident, tp);
                return new_Node(ND_NULL);
            }
            case SM_STATIC: 
            case SM_NONE: {
                if (consume(";")) {// extern宣言
                    ExVar *var = add_exvar(ident, tp);
                    return new_Node(ND_NULL);
                }
                RootineNode *rnode;
                GVar *var = add_gvar(ident, tp, mode == SM_STATIC);
                VarNode *args;
                Node *block;
                {
                    lvar_manager_PushScope(locals);
                    args = CreateArgs(tp->params);
                    block = compound_stmt();
                    lvar_manager_PopScope(locals);
                }
                rnode =new_RootineNode((Var*)var, args, block);
                rnode->total_offset = lvar_manager_GetTotalOffset(locals);

                return (Node *)rnode;
            }
        }
    } else {
        switch (mode) {
            case SM_TYPEDEF: {
                Type *alias = new_Alias(tp, ident->str, ident->len);
                regist_type(alias);
                return new_Node(ND_NULL);
            }
            case SM_EXTERN: {
                expect(';');
                ExVar *var = add_exvar(ident, tp);
                return new_Node(ND_NULL);
            }
            case SM_AUTO:
            case SM_REGISTER: {
                error_at(ident->str,
                         "globalセクションでローカルな宣言はできません。");
            }
            case SM_STATIC: {
                if(tp->kind == TY_FUNCTION){
                    expect(';');
                    ExVar *var = add_exvar(ident, tp);
                    return new_Node(ND_NULL);
                }
                GVar *var = add_gvar(ident, tp, true);
                Node *value = NULL;
                if (consume("=")) {  // 初期化
                    value = initilizer();
                }
                expect(';');
                VarInitNode *vnode = new_VarInitNode((Var *)var, value);
                return (Node *)vnode;
            }
            case SM_NONE: {
                if(tp->kind == TY_FUNCTION){
                    expect(';');
                    ExVar *var = add_exvar(ident, tp);
                    return new_Node(ND_NULL);
                }
                GVar *var = add_gvar(ident, tp, false);
                Node *value = NULL;
                if (consume("=")) {  // 初期化
                    value = initilizer();
                }
                expect(';');
                VarInitNode *vnode = new_VarInitNode((Var *)var, value);
                return (Node *)vnode;
            }
        }
    }
}
bool CanbeFuncDef(Type *tp) {
    if (tp->kind != TY_FUNCTION) return false;
    for (Param *par = tp->params->root; par; par = par->next) {
        if (par->token == NULL) return false;
    }
    return true;
}

bool declaration_specifier(StorageMode *mode, Type **base) {
    flag_n qualify_flag = 0,tmp;
    StorageMode sm;
    Type *type_specify;
    //初期化
    *mode = SM_NONE;
    *base = NULL;
    bool updated = true;
    int count = 0;
    while (updated) {
        updated = false;
        if ((sm = storage_specifier()) != SM_NONE) {
            if (*mode != SM_NONE && *mode != sm)
                error_here(false, "宣言修飾子は二つ以上宣言できません。");
            *mode = sm;
            updated = true;
        }
        if ((type_specify = type_specifier()) != NULL) {
            if (*base != NULL)
                error_here(false, "宣言において型は一意である必要があります。");
            *base = type_specify;
            updated = true;
        }
        if ((tmp = type_qualifier()) != 0) {
            qualify_flag |= tmp;
            updated = true;
        }
        count++;
    }
    if (count <= 1) return false;

    if (*base == NULL) error_here(false, "ベースの型が宣言されていません");
    (*base)->isConst = qualify_flag & ISCONST;
    (*base)->isVolatile = qualify_flag & ISVOLATILE;
    return true;
}
Node *initilizer() {
    if (consume("{")) {
        BlockNode *set = new_BlockNode(ND_SET);
        Node anker;
        anker.next = NULL;
        Node *top = &anker;
        while (!consume("}")) {
            top->next = assignment_expr();
            top = top->next;
            consume(",");
        }
        top->next = NULL;
        set->block = anker.next;
        return (Node *)set;
    }
    return assignment_expr();
}
Node *declaration_or_expr(){
    Node *nd = local_declaration(true);
    if(nd==NULL)nd = expression();
    return nd;
}

Node *constant_expr() { return condition_expr(); }
Node *condition_expr() {
    Node *nd = logical_or_expr();
    if (consume("?")) {
        CondNode *cnode = new_CondNode(ND_IFEL, nd, expression(), NULL);
        expect(':');
        cnode->F = condition_expr();
        nd = (Node *)cnode;
    }
    return nd;
}
Node *logical_or_expr() {
    Node *nd = logical_and_expr();
    while (consume("||")) {
        nd = (Node *)new_BinaryNode(ND_LGCOR, nd, logical_and_expr());
    }
    return nd;
}
Node *logical_and_expr() {
    Node *nd = or_expr();
    while (consume("&&")) {
        nd = (Node *)new_BinaryNode(ND_LGCAND, nd, or_expr());
    }
    return nd;
}
Node *or_expr() {
    Node *nd = xor_expr();
    while (consume("|")) {
        nd = (Node *)new_BinaryNode(ND_OR, nd, xor_expr());
    }
    return nd;
}
Node *xor_expr() {
    Node *nd = and_expression();
    while (consume("^")) {
        nd = (Node *)new_BinaryNode(ND_XOR, nd, and_expression());
    }
    return nd;
}
Node *and_expression() {
    Node *nd = equality_expr();
    while (consume("&")) {
        nd = (Node *)new_BinaryNode(ND_AND, nd, equality_expr());
    }
    return nd;
}
Node *equality_expr() {
    Node *nd = relational_expr();
    while (1) {
        if (consume("!="))
            nd = (Node *)new_BinaryNode(ND_NEQ, nd, relational_expr());
        else if (consume("=="))
            nd = (Node *)new_BinaryNode(ND_EQU, nd, relational_expr());
        else
            return nd;
    }
}
Node *relational_expr() {
    Node *nd = shift_expr();
    while (1) {
        if (consume("<"))
            nd = (Node *)new_BinaryNode(ND_GRT, nd, shift_expr());
        else if (consume("<="))
            nd = (Node *)new_BinaryNode(ND_GOE, nd, shift_expr());
        else if (consume(">"))
            nd = (Node *)new_BinaryNode(ND_GRT, shift_expr(), nd);
        else if (consume(">="))
            nd = (Node *)new_BinaryNode(ND_GOE, shift_expr(), nd);
        else
            return nd;
    }
}
Node *shift_expr() {
    Node *nd = add_expr();
    while (1) {
        if (consume("<<"))
            nd = (Node *)new_BinaryNode(ND_LSHFT, nd, add_expr());
        else if (consume(">>"))
            nd = (Node *)new_BinaryNode(ND_RSHFT, nd, add_expr());
        else
            return nd;
    }
}
Node *add_expr() {
    Node *nd = mul_expr();
    while (1) {
        if (consume("+"))
            nd = (Node *)new_BinaryNode(ND_ADD, nd, mul_expr());
        else if (consume("-"))
            nd = (Node *)new_BinaryNode(ND_SUB, nd, mul_expr());
        else
            return nd;
    }
}
Node *mul_expr() {
    Node *nd = cast_expr();
    while (1) {
        if (consume("*"))
            nd = (Node *)new_BinaryNode(ND_MUL, nd, cast_expr());
        else if (consume("/"))
            nd = (Node *)new_BinaryNode(ND_DIV, nd, cast_expr());
        else if (consume("%"))
            nd = (Node *)new_BinaryNode(ND_MOD, nd, cast_expr());
        else
            return nd;
    }
}
Node *cast_expr() {
    if (consume("(")) {
        Type *tp;
        if (!(tp = type_name(true))) {
            unconsume();
            return unary_expr();
        }
        expect(')');
        return (Node *)new_CastNode(tp, cast_expr());
    }
    return unary_expr();
}
Node *unary_expr() {
    // unary-oeperator
    if (consume("&")) return (Node *)new_UnaryNode(ND_ADDR, cast_expr());
    if (consume("*")) return (Node *)new_UnaryNode(ND_DEREF, cast_expr());
    if (consume("+")) return cast_expr();
    if (consume("-"))
        return (Node *)new_BinaryNode(ND_SUB, (Node *)new_NumNode(0),
                                      cast_expr());
    if (consume("~")) return (Node *)new_UnaryNode(ND_NOT, cast_expr());
    if (consume("!")) return (Node *)new_UnaryNode(ND_LGCNOT, cast_expr());

    if (consume("sizeof"))
        return (Node *)new_NumNode(check_Type()
                                       ? type_name(false)->size
                                       : type_assign(unary_expr())->size);
    if (consume("++"))
        return (Node *)new_BinaryNode(ND_ADD, unary_expr(),
                                      (Node *)new_NumNode(1));
    if (consume("--"))
        return (Node *)new_BinaryNode(ND_SUB, unary_expr(),
                                      (Node *)new_NumNode(1));

    return postfix_expr();
}
Node *postfix_expr() {
    Node *nd = primary_expr();
    while (1) {
        // 配列要素への参照
        if (consume("[")) {
            Node *index = expression();  // TODO: 配列
            expect(']');
            // x[a] -> *(x+a)
            return (Node *)new_UnaryNode(
                ND_DEREF, (Node *)new_BinaryNode(ND_ADD, nd, index));
        } else if (consume("(")) {
            CallNode *cnd = new_CallNode(nd);
            Node *args = NULL;
            //　引数を***逆順***で格納
            while (!consume(")")) {
                consume(",");
                Node *arg = assignment_expr();
                arg->next = args;
                args = arg;
            }
            cnd->arg = args;
            return (Node *)cnd;
        }  // TODO: struct(値)のメンバへの参照
        else if (consume(".")) {
            Token *tk = expect_ident();
        }  // TODO: struct pointer からのメンバへの参照
        else if (consume("->")) {
            Token *tk = expect_ident();
        } else if (consume("++"))
            nd = (Node *)new_BinaryNode(ND_INCRE, nd,
                                        (Node *)new_NumNode(1));  // x++
        else if (consume("--"))
            nd = (Node *)new_BinaryNode(ND_DECRE, nd,
                                        (Node *)new_NumNode(1));  // x--
        else
            return nd;
    }
}
Node *primary_expr() {
    if (consume("(")) {  // ( expr)の場合
        Node *node = expression();
        expect(')');
        return node;
    }
    Token *tk;
    if (tk = consume_string()) {
        CVar *var = add_CStr(tk->str, tk->len);
        Node *nd = (Node *)new_ConstNode(var);
        return nd;
    }
    // 関数や変数
    if (tk = consume_ident()) {
        Var *var = get_Var(tk);
        VarNode *vnd = new_VarNode(var);
        return (Node *)vnd;
    }
    return constant();
}
Node *constant() {
    Token *tk;
    if (tk = consume_integer()) return (Node *)new_NumNode(tk->val);
    if (tk = consume_float()) return (Node *)new_FloatNode(tk->val);
    if (tk = consume_char()) return (Node *)new_CharNode(*(tk->str));
    if (tk = consume_enum()) return (Node *)new_EnumNode(tk->str, tk->len);

    error_here(true, "不明なトークンです。");
}

Node *expression() {
    Node *node = assignment_expr();
    if(consume(",")){
        BlockNode *bnd = new_BlockNode(ND_SET);

        Node *top = node;
        do {
            top->next = assignment_expr();
            top = top->next;
        } while (consume(","));
        bnd->block = node;
        return (Node *)bnd;
    }
    return node;
}

Node *assignment_expr() {
    Node *nd = condition_expr();
    if (consume("="))
        return (Node *)new_BinaryNode(ND_ASSIGN, nd, assignment_expr());
    if (consume("+="))
        return (Node *)new_BinaryNode(ND_ADDASS, nd, assignment_expr());
    if (consume("-="))
        return (Node *)new_BinaryNode(ND_SUBASS, nd, assignment_expr());
    if (consume("*="))
        return (Node *)new_BinaryNode(ND_MULASS, nd, assignment_expr());
    if (consume("/="))
        return (Node *)new_BinaryNode(ND_DIVASS, nd, assignment_expr());
    if (consume("%="))
        return (Node *)new_BinaryNode(ND_MODASS, nd, assignment_expr());
    if (consume("<<="))
        return (Node *)new_BinaryNode(ND_LSHASS, nd, assignment_expr());
    if (consume(">>="))
        return (Node *)new_BinaryNode(ND_RSHASS, nd, assignment_expr());
    if (consume("&="))
        return (Node *)new_BinaryNode(ND_ANDASS, nd, assignment_expr());
    if (consume("|="))
        return (Node *)new_BinaryNode(ND_ORASS, nd, assignment_expr());
    if (consume("^="))
        return (Node *)new_BinaryNode(ND_XORASS, nd, assignment_expr());
    return nd;
}

Node *translation_unit() {
    Node anker;
    anker.next = NULL;
    Node *top = &anker;
    while (!at_eof()) {
        top->next = external_declaration();
        top = top->next;
    }
    top->next = NULL;

    return anker.next;
}
Node *external_declaration() { return global_declaration(); }

Node *compound_stmt() {
    BlockNode *bnode = new_BlockNode(ND_BLOCK);
    BlockNode *set = new_BlockNode(ND_SET);

    Node anker;
    anker.next = NULL;
    Node *top = &anker, *node;
    if (!consume("{")) return NULL;
    //新たなスコープ追加
    lvar_manager_PushScope(locals);

    while (!consume("}")) {
        node = local_declaration(false);
        if (node == NULL) node = statement();
        top->next = node;
        top = top->next;
    }
    top->next = NULL;

    set->block = anker.next;
    bnode->block = (Node *)set;
    //スコープ除去
    lvar_manager_PopScope(locals);

    return (Node *)bnode;
}
Node *statement() {
    Node *res;
    res = selection_stmt();
    if (res) return res;
    res = iteration_stmt();
    if (res) return res;
    res = jump_stmt();
    if (res) return res;
    res = labeled_stmt();
    if (res) return res;
    res = compound_stmt();
    if (res) return res;
    return expression_stmt();
}
Node *labeled_stmt() {
    if (consume("case")) {
        Node *label = constant_expr();
        expect(':');
        Node *nd = statement();
        return (Node *)new_BinaryNode(ND_CASE, label, nd);
    }
    if (consume("default")) {
        expect(':');
        Node *nd = statement();
        return (Node *)new_UnaryNode(ND_DEFAULT, nd);
    }
    if (check_ahead(":")) {
        Token *tk = expect_ident();
        expect(':');
        return (Node *)new_LabelNode(ND_LABEL, tk->str, tk->len);
    }
    return NULL;
}
Node *expression_stmt() {
    if (consume(";")) return new_Node(ND_NULL);
    Node *nd = expression();
    expect(';');
    return nd;
}
Node *selection_stmt() {
    if (consume("if")) {
        expect('(');
        Node *cond = expression();
        expect(')');
        Node *T = statement();
        if (consume("else")) {
            Node *F = statement();
            return (Node *)new_CondNode(ND_IFEL, cond, T, F);
        }
        return (Node *)new_CondNode(ND_IF, cond, T, NULL);
    }
    if (consume("switch")) {  // TODO: switch文の実装
        expect('(');
        Node *cond = expression();
        expect(')');
        return (Node *)new_CondNode(ND_SWITCH, cond, statement(), NULL);
    }
    return NULL;
}
Node *iteration_stmt() {
    if (consume("while")) {
        expect('(');
        Node *cond = expression();
        expect(')');
        Node *T = statement();
        return (Node *)new_CondNode(ND_WHILE, cond, T, NULL);
    }
    if (consume("do")) {
        Node *T = statement();
        expect_str("while");
        expect('(');
        Node *cond = expression();
        expect(')');
        expect(';');
        return (Node *)new_CondNode(ND_DOWHILE, cond, T, NULL);
    }
    if (consume("for")) {
        lvar_manager_PushScope(locals);
        expect('(');
        Node *init = check(";")? nullNode : declaration_or_expr();
        expect(';');
        Node *cond = check(";") ? nullNode : expression();
        expect(';');
        Node *update = check(")") ? nullNode : expression();
        expect(')');
        Node *block = statement();
        lvar_manager_PopScope(locals);
        return (Node *)new_ForNode(init, cond, update, block);
    }
    return NULL;
}
Node *jump_stmt() {
    if (consume("goto")) {
        Token *tk = expect_ident();
        expect(';');
        return (Node *)new_LabelNode(ND_GOTO, tk->str, tk->len);
    }
    if (consume("continue")) {
        expect(';');  // TODO: continue
                      // でジャンプルする先を指定しないといけない
        return (Node *)new_LabelNode(ND_CONTINUE, NULL, 0);
    }
    if (consume("break")) {
        expect(';');  // TODO: break でジャンプルする先を指定しないといけない
        return (Node *)new_LabelNode(ND_BREAK, NULL, 0);
    }
    if (consume("return")) {
        if (consume(";")) return (Node *)new_UnaryNode(ND_RETURN, NULL);
        Node *nd = expression();
        expect(';');
        return (Node *)new_UnaryNode(ND_RETURN, nd);
    }
    return NULL;
}