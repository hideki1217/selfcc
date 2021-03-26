#include <stdlib.h>

#include "selfcc.h"

#define ISCONST 1
#define ISVOLATILE 2

#define NOITEM -1

#define ISNULL(a) (a) == NULL
#define ISNNULL(a) (a) != NULL

LVar_Manager *locals;
CC_BidList *global_list;
CC_AVLTree *globals;
CC_AVLTree *externs;
CC_BidList *constants;

//文法部
static int structId = 0;
static int unionId = 0;
static int enumId = 0;
int LCcount = 0;
static CC_IntQueue *continue_index;
static CC_IntQueue *break_index;
int Lcount = 0;
Node *nullNode;

void initialize_parser() {
    continue_index = cc_intqueue_new();
    break_index = cc_intqueue_new();
}

#define new_Params() cc_vector_new()
void continue_push(int index);
void continue_pop();
/**
 * @brief  continueで飛べる最短のindexを返す。otherwise NOITEM
 * @note
 * @retval 最短のindex or NOITEM
 */
int continue_top();
void break_push(int index);
void break_pop();
/**
 * @brief  breakで飛べる最短のindexを返す。otherwise NOITEM
 * @note
 * * @retval 最短のindex or NOITEM
 */
int break_top();

void declarator(TypeModel *model, Token **ident) {
    *ident = NULL;
    while (consume("*")) {
        tpmodel_addptr(model);
        if (ISNNULL(consume("const"))) tpmodel_setconst(model);
        if (ISNNULL(consume("volatile"))) tpmodel_setvltle(model);
        // error_here(true, "型を修飾する語でなければなりません。");
    }
    *ident = direct_declarator(model);
}
Token *direct_declarator(TypeModel *base) {
    TypeModel inner_model = {NULL};
    Token *identName = NULL;
    if (consume("(")) {
        declarator(&inner_model, &identName);
        expect(")");
    } else
        identName = consume_ident();
    // if (tk == NULL) error_here(false, "宣言すべき識別子が存在しません。");
    if (consume("[")) {
        int size = expect_integer();  // TODO:　定数式ならおっけ
        tpmodel_addarray(base, size);
        expect("]");
    }
    if (consume("(")) {
        tpmodel_addfunc(base);

        while (!consume(")")) {
            consume(",");
            if (consume("...")) {
                tpmodel_addvaarg(base);
                expect(")");
                break;
            }
            TypeModel prm_tpmodel = {NULL};
            Token *prm_name = NULL;
            // 引数の型名を読む
            StorageMode mode;
            if (!declaration_specifier(&mode, &prm_tpmodel))
                error_here(true, "型名が存在しません");
            declarator(&prm_tpmodel, &prm_name);

            tpmodel_addprm(base, prm_tpmodel.type, prm_name);
        }
    }
    // ITYPE1->ITYPE2->anker , BASE
    //    => ITYPE1->ITYPE2->BASE
    Type *res = base->type;
    base->type = inner_model.type;
    tpmodel_setbase(base, res);

    return identName;
}
bool specifier_qualifier(TypeModel *model, bool errorExpected) {
    bool updated = true;
    bool type_specified = false;
    flag_n flag = 0;
    int count = 0;  // 更新が行われたかどうかを示すカウンタ >1 なら更新あり
    while (updated) {
        if (type_specifier(model)) {
            if (type_specified)
                error_here(false, "宣言において型は一意である必要があります。");
            updated = true;
            type_specified = true;
        }
        if (type_qualifier(&flag)) {
            updated = true;
        }
        updated = false;
        count++;
    }
    if (!errorExpected & count <= 1)
        return false;  // 更新がない場合(errorExpectedが指定されていれば無視する)
    if (!type_specified) error_here(false, "ベースの型が宣言されていません");

    if(flag | ISCONST)tpmodel_setconst(model);
    if(flag | ISVOLATILE)tpmodel_setvltle(model);

    return count <= 1;
}
Type *type_naming(bool isCheck) {
    TypeModel model = {NULL};
    if (isCheck & !specifier_qualifier(&model, !isCheck)) return NULL;
    abstract_declarator(&model);
    return model.type;
}
void abstract_declarator(TypeModel *model) {
    bool isConst = false;
    bool isVolatile = false;
    TypeModel inner_model = {NULL};
    Token *tk = NULL;
    while (consume("*")) {
        tpmodel_addptr(model);
        if (ISNNULL(consume("const"))) tpmodel_setconst(model);
        if (ISNNULL(consume("volatile"))) tpmodel_setvltle(model);
    }
    if (consume("(")) {
        abstract_declarator(&inner_model);
        expect(")");
    }
    if (tk != NULL)
        error_here(
            false,
            "ここに識別子は存在してはいけません");  // TODO:
                                                    // 識別子を読み取る場所を用意すべし
    if (consume("[")) {
        int size = expect_integer();  // TODO:　定数式ならおっけ
        tpmodel_addarray(model, size);
        expect("]");
    }
    if (consume("(")) {
        tpmodel_addfunc(model);

        while (!consume(")")) {
            consume(",");
            if (consume("...")) {
                tpmodel_addvaarg(model);
                expect(")");
                break;
            }
            Type *now_type;
            Token *prm_name;
            now_type = type_naming(false);
            prm_name = consume_ident();
            tpmodel_addprm(model, now_type, prm_name);
        }
        expect(")");
    }
    Type *tmp = model->type;
    model->type = inner_model.type;
    tpmodel_setbase(model, tmp);
}

StorageMode storage_specifier() {
    if (consume("typedef")) return SM_TYPEDEF;
    if (consume("extern")) return SM_EXTERN;
    if (consume("static")) return SM_STATIC;
    if (consume("auto")) return SM_AUTO;
    if (consume("register")) return SM_REGISTER;
    return SM_NONE;
}
bool type_specifier(TypeModel *model) {  // TODO: 未実装
    // struct宣言
    if (consume("struct")) {
        return true;
    }
    // union宣言
    if (consume("union")) {
        return true;
    }
    // enum宣言
    if (consume("enum")) {
        return true;
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
        return true;
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
        return true;
    }
    Type *tp;
    if (consume_Type(&tp)) {
        model->type = tp;
        return true;
    }

    return false;
}
bool type_qualifier(flag_n *flag) {
    bool update = false;
    bool con =false,vol = false;
    while (1) {
        if (consume("const"))
            con |= true, update = true;
        else if (consume("volatile"))
            vol |= true, update = true;
        else
            break;
    }
    *flag |= con*ISCONST + vol *ISVOLATILE;
    return update;
}
Node *local_declaration(bool /*式と同列に扱うか？*/ asExpr) {
    StorageMode mode = SM_NONE;
    TypeModel model = {NULL};
    Token *ident;
    if (!declaration_specifier(&mode, &model)) return NULL;

    BlockNode *set = new_BlockNode(ND_SET);
    Node anker, *top = &anker;
    do {
        Type *type = new_Type(TYPE_PARAMETERS(model.type));
        TypeModel clone_model = { type };
        declarator(&clone_model, &ident);
        Node *res;
        if (CanbeFuncDef(clone_model.type)) {
            switch (mode) {
                case SM_AUTO:
                case SM_REGISTER:
                case SM_TYPEDEF:
                    error_at(ident->str,
                             "関数に対して無効なストレージ クラスです");
                case SM_EXTERN:
                case SM_NONE: {
                    ExVar *var = add_exvar(ident, clone_model.type);
                    res = new_Node(ND_NULL);
                    break;
                }
                case SM_STATIC:
                    error_at(
                        ident->str,
                        "blockscopeではextern修飾子のみが許可されています。");
            }
        } else {
            switch (mode) {
                case SM_TYPEDEF: {
                    typemgr_reg(ident->str,ident->len,clone_model.type);
                    res = new_Node(ND_NULL);
                    break;
                }
                case SM_EXTERN: {
                    ExVar *var = add_exvar(ident, clone_model.type);
                    res = new_Node(ND_NULL);
                    break;
                }
                case SM_AUTO:
                case SM_REGISTER:
                case SM_NONE: {
                    LVar *var = add_lvar(ident, clone_model.type);
                    Node *value = NULL;
                    if (consume("=")) {  // 初期化
                        value = initilizer();
                    }
                    res = (Node *)new_VarInitNode((Var *)var, value);
                    break;
                }
                case SM_STATIC: {
                    GVar *var = add_gvar(ident, clone_model.type, true);
                    Node *value = NULL;
                    if (consume("=")) {  // 初期化
                        value = initilizer();
                    }
                    res = (Node *)new_VarInitNode((Var *)var, value);
                    break;
                }
            }
        }
        top->next = res;
        top = res;
    } while (consume(","));
    top->next = NULL;
    set->block = anker.next;
    if (!asExpr) expect(";");

    return (Node *)set;
}
/**
 * @brief  宣言付きのParamsからVarNodeのlistを作成
 * 可変長引数については受け手の関数が対応するので、
 * この関数では無視する。
 * @note
 * @param  *params: 対象のParams
 * @retval VarNodeのlistの先頭(Paramsと同順)
 */
VarNode *CreateArgs(Params *params) {
    VarNode anker;
    anker.base.next = NULL;
    VarNode *top = &anker, *tmp;
    for(int i=0;i<params->size;i++){
        Param *par = params->_[i].ptr;
        if(par->kind == PA_VAARG)break;
        if (par->token == NULL) error("引数の識別子が存在しません。");
        Var *var = (Var *)add_lvar(par->token, par->type);// 登録
        tmp = new_VarNode(var); // VarNode生成
        top->base.next = (Node *)tmp;
        top = tmp;
    }
    return (VarNode *)anker.base.next;
}
Node *global_declaration() {
    StorageMode mode = SM_NONE;
    TypeModel model = {NULL};
    Token *ident;

    if (!declaration_specifier(&mode, &model)) return NULL; // 更新がなかった場合
    BlockNode *set = new_BlockNode(ND_SET);
    Node anker, *top = &anker;
    do {
        Type *type = new_Type(TYPE_PARAMETERS(model.type));
        TypeModel clone_model = { type };
        declarator(&clone_model, &ident);

        Node *res;
        if (CanbeFuncDef(clone_model.type)) {
            lvar_manager_Clear(locals); // ローカル変数リストを初期化
            switch (mode) {
                case SM_AUTO:
                case SM_REGISTER:
                case SM_TYPEDEF:
                    error_at(ident->str,
                             "関数定義が不正な修飾子を宣言しています。");
                case SM_EXTERN: {
                    ExVar *var = add_exvar(ident, clone_model.type);
                    res = new_Node(ND_NULL);
                    break;
                }
                case SM_STATIC:
                case SM_NONE: {
                    if (!check("{")) {  // extern宣言
                        ExVar *var = add_exvar(ident, clone_model.type);
                        res = new_Node(ND_NULL);
                        break;
                    }
                    RootineNode *rnode;
                    GVar *var = add_gvar(ident, clone_model.type, mode == SM_STATIC);
                    VarNode *args;
                    Node *block;
                    {
                        lvar_manager_PushScope(locals);
                        args = CreateArgs(type_params(clone_model.type));
                        block = compound_stmt();
                        lvar_manager_PopScope(locals);
                    }
                    rnode = new_RootineNode((Var *)var, args, block);
                    rnode->total_offset = lvar_manager_GetTotalOffset(locals);

                    top->next = (Node *)rnode;
                    top = (Node *)rnode;
                    //関数定義が来たらもう終わり。
                    top->next = NULL;
                    set->block = anker.next;
                    return (Node *)set;
                }
            }
        } else {
            switch (mode) {
                case SM_TYPEDEF: {
                    typemgr_reg(ident->str,ident->len,clone_model.type);
                    res = new_Node(ND_NULL);
                    break;
                }
                case SM_EXTERN: {
                    ExVar *var = add_exvar(ident, clone_model.type);
                    res = new_Node(ND_NULL);
                    break;
                }
                case SM_AUTO:
                case SM_REGISTER: {
                    error_at(ident->str,
                             "globalセクションでローカルな宣言はできません。");
                }
                case SM_STATIC: {
                    if (type_isfunc(clone_model.type)) {
                        ExVar *var = add_exvar(ident, clone_model.type);
                        res = new_Node(ND_NULL);
                        break;
                    }
                    GVar *var = add_gvar(ident, clone_model.type, true);
                    Node *value = NULL;
                    if (consume("=")) {  // 初期化
                        value = initilizer();
                    }
                    res = (Node *)new_VarInitNode((Var *)var, value);
                    break;
                }
                case SM_NONE: {
                    if (type_isfunc(clone_model.type)) {
                        ExVar *var = add_exvar(ident, clone_model.type);
                        res = new_Node(ND_NULL);
                        break;
                    }
                    GVar *var = add_gvar(ident, clone_model.type, false);
                    Node *value = NULL;
                    if (consume("=")) {  // 初期化
                        value = initilizer();
                    }
                    res = (Node *)new_VarInitNode((Var *)var, value);
                    break;
                }
            }
        }
        top->next = res;
        top = res;
    } while (consume(","));
    expect(";");

    top->next = NULL;
    set->block = anker.next;

    return (Node *)set;
}
bool CanbeFuncDef(Type *tp) {
    if (!(type_isfunc(tp))) return false;
    for(int i=0;i<type_params(tp)->size;i++){
        Param *par = type_params(tp)->_[i].ptr;
        if(par->token == NULL)return false;
    }
    return true;
}

bool declaration_specifier(StorageMode *mode, TypeModel *model) {
    StorageMode sm;
    //初期化
    *mode = SM_NONE;
    model->type = NULL;
    bool updated = true;
    bool type_specified = false;
    flag_n flag = 0;
    int count = 0;
    while (updated) {
        updated = false;
        if ((sm = storage_specifier()) != SM_NONE) {
            if (*mode != SM_NONE && *mode != sm)
                error_here(false, "宣言修飾子は二つ以上宣言できません。");
            *mode = sm;
            updated = true;
        }
        if (type_specifier(model)) {
            if (type_specified)
                error_here(false, "宣言において型は一意である必要があります。");
            type_specified = true;
            updated = true;
        }
        if (type_qualifier(&flag)) {
            updated = true;
        }
        count++;
    }
    if (count <= 1) return false; // 更新がなかった場合

    if (!type_specified) error_here(false, "ベースの型が宣言されていません");

    if(flag & ISCONST)tpmodel_setconst(model);
    if(flag & ISVOLATILE)tpmodel_setvltle(model);
    
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
Node *declaration_or_expr() {
    Node *nd = local_declaration(true);
    if (nd == NULL) nd = expression();
    return nd;
}

Node *constant_expr() { return condition_expr(); }
Node *condition_expr() {
    Node *nd = logical_or_expr();
    if (consume("?")) {
        CondNode *res;
        int lcount = Lcount++;
        {
            Node *T = expression();
            expect(":");
            Node *F = condition_expr();
            res = new_CondNode(ND_IFEL, nd, T, F);
        }
        res->index = lcount;
        return (Node *)res;
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
        if (!(tp = type_naming(true))) {
            unconsume();
            return unary_expr();
        }
        expect(")");
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
                                       ? type_size(type_naming(false))
                                       : type_size(type_assign(unary_expr())) );
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
            expect("]");
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
        expect(")");
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
    if (consume(",")) {
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
        expect(":");
        Node *nd = statement();
        return (Node *)new_BinaryNode(ND_CASE, label, nd);
    }
    if (consume("default")) {
        expect(":");
        Node *nd = statement();
        return (Node *)new_UnaryNode(ND_DEFAULT, nd);
    }
    if (check_ahead(":")) {
        Token *tk = expect_ident();
        expect(":");
        // return (Node *)new_LabelNode(ND_LABEL, tk->str, tk->len);
        error("まだlabelを作る機構は未完成だよう");
    }
    return NULL;
}
Node *expression_stmt() {
    if (consume(";")) return new_Node(ND_NULL);
    Node *nd = expression();
    expect(";");
    return nd;
}
Node *selection_stmt() {
    int lcount;  // 識別番号
    if (consume("if")) {
        CondNode *res;
        lcount = Lcount++;
        {
            expect("(");
            Node *cond = expression();
            expect(")");
            Node *T = statement();
            if (consume("else")) {
                Node *F = statement();
                res = new_CondNode(ND_IFEL, cond, T, F);
            } else
                res = new_CondNode(ND_IF, cond, T, NULL);
        }
        res->index = lcount;
        return (Node *)res;
    }
    if (consume("switch")) {  // TODO: switch文の実装
        CondNode *res;
        lcount = Lcount++;
        break_push(lcount);
        {
            expect("(");
            Node *cond = expression();
            expect(")");
            res = new_CondNode(ND_SWITCH, cond, statement(), NULL);
        }
        break_pop();
        res->index = lcount;
        return (Node *)res;
    }
    return NULL;
}
Node *iteration_stmt() {
    int lcount;  // 識別番号
    if (consume("while")) {
        CondNode *res;
        lcount = Lcount++;
        continue_push(lcount);
        break_push(lcount);
        {
            expect("(");
            Node *cond = expression();
            expect(")");
            Node *T = statement();
            res = new_CondNode(ND_WHILE, cond, T, NULL);
        }
        continue_pop();
        break_pop();
        res->index = lcount;
        return (Node *)res;
    }
    if (consume("do")) {
        CondNode *res;
        lcount = Lcount++;
        continue_push(lcount);
        break_push(lcount);
        {
            Node *T = compound_stmt();
            expect("while");
            expect("(");
            Node *cond = expression();
            expect(")");
            expect(";");
            res = new_CondNode(ND_DOWHILE, cond, T, NULL);
        }
        continue_pop();
        break_pop();
        res->index = lcount;
        return (Node *)res;
    }
    if (consume("for")) {
        ForNode *res;
        lcount = Lcount++;
        continue_push(lcount);
        break_push(lcount);
        lvar_manager_PushScope(locals);
        {
            expect("(");
            Node *init = check(";") ? nullNode : declaration_or_expr();
            expect(";");
            Node *cond = check(";") ? nullNode : expression();
            expect(";");
            Node *update = check(")") ? nullNode : expression();
            expect(")");
            Node *block = statement();
            res = new_ForNode(init, cond, update, block);
        }
        continue_pop();
        break_pop();
        lvar_manager_PopScope(locals);
        res->index = lcount;
        return (Node *)res;
    }
    return NULL;
}
Node *jump_stmt() {
    int jumpTo;
    Token *token;
    if (consume("goto")) {  // TODO: gotoを実装
        Token *tk = expect_ident();
        expect(";");
        // return (Node *)new_LabelNode(ND_GOTO, tk->str, tk->len);
        error("gotoは未対応だよう");
    }
    if (token = consume("continue")) {
        expect(";");
        if ((jumpTo = continue_top()) != NOITEM)
            return (Node *)new_LabelNode(ND_CONTINUE, jumpTo);
        else
            error_at(token->str,
                     "continueはfor,while,do-while文の中でだけ使えます");
    }
    if (consume("break")) {
        expect(";");
        if ((jumpTo = break_top()) != NOITEM)
            return (Node *)new_LabelNode(ND_BREAK, jumpTo);
        else
            error_at(token->str,
                     "breakはfor,while,do-while文の中でだけ使えます");
    }
    if (consume("return")) {
        if (consume(";")) return (Node *)new_UnaryNode(ND_RETURN, NULL);
        Node *nd = expression();
        expect(";");
        return (Node *)new_UnaryNode(ND_RETURN, nd);
    }
    return NULL;
}

//////////////ヘルパー関数
void continue_push(int index) { cc_intqueue_push(continue_index, index); }
void continue_pop() { cc_intqueue_pop(continue_index); }
int continue_top() {
    int x;
    if (cc_intqueue_top(continue_index, &x)) return x;
    return NOITEM;
}
void break_push(int index) { cc_intqueue_push(break_index, index); }
void break_pop() { cc_intqueue_pop(break_index); }
int break_top() {
    int x;
    if (cc_intqueue_top(break_index, &x)) return x;
    return NOITEM;
}