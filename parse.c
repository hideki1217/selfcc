#include <stdlib.h>

#include "selfcc.h"

//文法部
Node *code;
int Lcount = 0;
int LCcount = 0;
Node *nullNode;

extern int aho(int x);

void program() {
    Node head;
    head.next = NULL;
    Node *node = &head;
    while (!at_eof()) {
        node->next = rootine();
        node = node->next;
    }
    node->next = NULL;
    code = head.next;
}
Node *rootine() {
    // TODO:
    // mapをqueueにぶちこんで管理するBlockに入るたびにpushしていき出るときにpop
    cc_map_for_var_clear(locals);  // ローカル変数をrootineごとにリセット

    flag_n flag = storage_specifier();

    if (IsExtern(flag)) return extern_declaration(flag);

    Type *base_type = type_name();
    Token *name = expect_ident();
    if (consume("(")) {  // 関数定義
        RootineNode *node = new_RootineNode(name->str, name->len,
                                            base_type->name, base_type->len);
        Params *params=new_Params();
        Node anker;
        anker.next = NULL;
        Node *top = &anker;
        Type *now_type;
        while (!consume(")")) {
            consume(",");

            now_type = expect_type();
            LVar *var = add_lvar(expect_var(), now_type, flag);
            cc_map_for_var_add(locals, var->base.name, var->base.len, var);

            params_addParam(params,now_type);
            top->next = (Node *)new_VarNode((Var *)var);
            top = top->next;
        }
        node->arg = (VarNode *)(anker.next);
        Type *func = new_Function(base_type, params);
        // 関数も定義されたら、グローバル変数として扱う。
        add_gvar(name, func, flag);

        node->block = stmt();
        node->total_offset = locals->offset;

        return (Node *)node;
    } else {  //グローバル変数
        if (consume("[")) {
            int size = expect_integer();
            expect(']');
            base_type = new_Array(base_type, size);
        }
        GVar *var = add_gvar(name, base_type, flag);
        Node *value = NULL;
        if (consume("=")) {  // TODO: グローバル変数の初期化未対応
            error_at(name->str, "グローバル変数の初期化は未対応です");
        }
        expect(';');
        return (Node *)new_VarInitNode((Var *)var, value);
    }
}
Node *extern_declaration(flag_n flag) {
    Type *base_type = type_name();
    Token *name = expect_ident();
    Node *node = new_Node(ND_NULL);
    if (consume("(")) {  // 関数定義
        Params *params= new_Params();
        Type *now_type;
        Token *tk;
        while (!consume(")")) {
            consume(",");
            if (consume("...")) {
                params_addVaArg(params);
                expect(')');
                break;
            }
            now_type = type_name();
            consume_ident(&tk);
            params_addParam(params,now_type);
        }
        Type *func = new_Function(base_type, params);
        add_gvar(name, func, flag);
    } else {  //グローバル変数
        if (consume("[")) {
            int size = expect_integer();
            expect(']');
            base_type = new_Array(base_type, size);
        }
        add_gvar(name, base_type, flag);
    }
    expect(';');
    return node;
}
Node *stmt() {
    if (consume("{")) {
        BlockNode *node = new_BlockNode(ND_BLOCK);

        BlockNode *set = new_BlockNode(ND_SET);
        node->block = (Node *)set;

        Node anker;
        anker.next = NULL;
        Node *top = &anker;
        while (!consume("}")) {
            top->next = stmt();
            top = top->next;
        }
        top->next = NULL;
        set->block = anker.next;

        return (Node *)node;
    }
    if (consume("return")) {
        BinaryNode *node = new_BinaryNode(ND_RETURN, expr(), NULL);
        expect(';');
        return (Node *)node;
    }
    if (consume("if")) {
        expect('(');
        Node *condition = expr();
        expect(')');
        Node *A = stmt();

        CondNode *node;
        if (consume("else")) {
            node = new_CondNode(ND_IFEL, condition, A, stmt());
        } else {
            node = new_CondNode(ND_IF, condition, A, NULL);
        }
        return (Node *)node;
    }
    if (consume("while")) {
        expect('(');
        Node *condition = expr();
        expect(')');
        Node *A = stmt();

        CondNode *node = new_CondNode(ND_WHILE, condition, A, NULL);
        return (Node *)node;
    }
    if (consume("for")) {
        expect('(');
        Node *init = check(";") ? nullNode : expr();
        expect(';');
        Node *cond = check(";") ? nullNode : expr();
        expect(';');
        Node *update = check(")") ? nullNode : expr();
        expect(')');

        ForNode *node = new_ForNode(init, cond, update, stmt());

        return (Node *)node;
    }
    Node *node = expr();
    expect(';');
    return node;
}
Node *expr() {
    flag_n flag =
        storage_specifier();  // TODO: エラーになる場合がるからそれを実装
    if (check_Type()) {
        Type *type = type_name();

        Token *tk = expect_var_not_proceed();
        if (token_ismutch(tk->next, "[", 1)) {  //配列の場合
            consume_ident(&tk);
            consume("[");
            int size = expect_integer();  // TODO:
                                          // 定数値を実装し次第それも入れるべし
            expect(']');
            type = new_Array(type, size);
            LVar *var = add_lvar(tk, type, flag);

            return (Node *)new_VarInitNode(
                (Var *)var,
                NULL);  // TODO: 配列初期化じの初期値を受けるならNULLを解除
        } else {  //配列でない場合
            LVar *var = add_lvar(tk, type, flag);
            return assign();
        }
    }
    return assign();
}
Node *assign() {
    Node *node = equality();
    if (consume("=")) return (Node *)new_BinaryNode(ND_ASSIGN, node, assign());
    if (consume("+=")) return (Node *)new_BinaryNode(ND_ADDASS, node, assign());
    if (consume("-=")) return (Node *)new_BinaryNode(ND_SUBASS, node, assign());
    if (consume("*=")) return (Node *)new_BinaryNode(ND_MULASS, node, assign());
    if (consume("/=")) return (Node *)new_BinaryNode(ND_DIVASS, node, assign());

    return node;
}
Node *equality() {
    Node *node = relational();

    while (1) {
        if (consume("==")) {
            node = (Node *)new_BinaryNode(ND_EQU, node, relational());
        } else if (consume("!=")) {
            node = (Node *)new_BinaryNode(ND_NEQ, node, relational());
        } else
            return node;
    }
}
Node *relational() {
    Node *node = add();

    while (1) {
        if (consume(">=")) {
            node = (Node *)new_BinaryNode(ND_GOE, add(), node);
        } else if (consume("<=")) {
            node = (Node *)new_BinaryNode(ND_GOE, node, add());
        } else if (consume(">")) {
            node = (Node *)new_BinaryNode(ND_GRT, add(), node);
        } else if (consume("<")) {
            node = (Node *)new_BinaryNode(ND_GRT, node, add());
        } else
            return node;
    }
}
Node *add() {
    Node *node = mul();

    while (1) {
        if (consume("+")) {
            node = (Node *)new_BinaryNode(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = (Node *)new_BinaryNode(ND_SUB, node, mul());
        } else
            return node;
    }
}
Node *mul() {
    Node *node = unary();

    while (1) {
        if (consume("*"))
            node = (Node *)new_BinaryNode(ND_MUL, node, unary());
        else if (consume("/"))
            node = (Node *)new_BinaryNode(ND_DIV, node, unary());
        else
            return node;
    }
}
Node *unary() {
    if (consume("+")) return primary();
    if (consume("-"))
        return (Node *)new_BinaryNode(ND_SUB, (Node *)new_NumNode(0),
                                      primary());
    if (consume("&")) return (Node *)new_UnaryNode(ND_ADDR, unary());
    if (consume("*")) return (Node *)new_UnaryNode(ND_DEREF, unary());
    if (consume("sizeof")) {
        return (Node *)new_NumNode(type_assign(unary())->size);
    }
    if (consume("++"))
        return (Node *)new_BinaryNode(ND_ADD, (Node *)new_NumNode(1),
                                      primary());  // ++x
    if (consume("--"))
        return (Node *)new_BinaryNode(ND_SUB, primary(),
                                      (Node *)new_NumNode(1));  // --x

    Node *node = primary();
    if (consume("++"))
        return (Node *)new_BinaryNode(ND_INCRE, node,
                                      (Node *)new_NumNode(1));  // x++
    if (consume("--"))
        return (Node *)new_BinaryNode(ND_DECRE, node,
                                      (Node *)new_NumNode(1));  // x--

    return node;
}
Node *primary() {
    Node *nd;

    if (consume("(")) {  // ( expr)の場合
        Node *node = expr();
        expect(')');
        nd = node;
    } else {
        Token *tk = consume_hard();
        switch (tk->kind) {
            case TK_NUM: {  // 数値の場合
                nd = (Node *)new_NumNode(tk->val);
                break;
            }
            case TK_STRING: {
                CVar *var = add_CStr(tk->str, tk->len);
                nd = (Node *)new_ConstNode(var);
                break;
            }
            case TK_IDENT: {
                Var *var = get_Var(tk);
                if (consume("(")) {  // 関数呼び出しの場合
                    if (var->type->kind != TY_FUNCTION)
                        error_at(tk->str,
                                 "関数のように呼び出すことはできません。");
                    CallNode *node = new_CallNode(var);
                    Node *args = NULL;
                    while (!consume(")")) {
                        consume(",");
                        Node *arg = add();
                        arg->next = args;
                        args = arg;
                    }
                    node->arg = args;
                    nd = (Node *)node;
                } else {  // 変数の場合
                    nd = (Node *)new_VarNode(var);
                }
                break;
            }
        }
    }
    if (consume("[")) {  // a[x]の形の時:: a[x] -> *(a+x)
        Node *index = add();
        expect(']');
        nd = (Node *)new_UnaryNode(ND_DEREF,
                                   (Node *)new_BinaryNode(ND_ADD, nd, index));
    }

    return nd;
}
Type *type_name() {
    bool flag_const = false;
    bool flag_volatile = false;
    Type *tp = NULL;
    while (!consume_Type(&tp)) {
        if (consume("const"))
            flag_const = true;
        else if (consume("volatile"))
            flag_volatile = true;
        else
            error_here(true, "型を修飾する語でなければなりません。");
    }
    abstract_declarator(&tp);
    return tp;
}
void abstract_declarator(Type **base) {  // TODO: 型の解析機を作らねばならない
    while (consume("*")) {
        *base = new_Pointer(*base);
        consume("const");
        consume("volatile");
    }
    if (consume("(")) {
        abstract_declarator(base);  // TODO: ネストした型を解析をすべし
    }
}
flag_n storage_specifier() {
    ////////定義フラグ
    bool isExtern = false;
    bool isStatic = false;
    bool isAuto = false;
    bool isRegister = false;
    /////////////////
    if (consume("typedef")) {
        return -1;
    }
    while (1) {
        if (consume("extern"))
            isExtern = true;
        else if (consume("static"))
            !isAuto ? isStatic = true
                    : error_here(false, "autoな変数をstaticにはできません。");
        else if (consume("auto"))
            !isStatic ? isAuto = true
                      : error_here(false, "staticな変数をautoにはできません。");
        else if (consume("register"))
            isRegister = true;
        else
            break;
    }
    return makeFlag(isExtern, isStatic, isAuto, isRegister);
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
    while ("^") {
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
        else if (consume("%%"))
            nd = (Node *)new_BinaryNode(ND_MOD, nd, cast_expr());
        else
            return nd;
    }
}
Node *cast_expr() {
    if (consume("(")) {
        Type *tp = type_name();
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
        return (Node *)new_NumNode(
            check_Type() ? type_name()->size : type_assign(unary_expr())->size);
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
            Node *index = expression();
            expect(']');
            return (Node *)new_UnaryNode(
                ND_DEREF, (Node *)new_BinaryNode(ND_ADD, nd, index));
        }  // TODO: 関数呼び出し
        else if (consume("(")) {
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
        Node *node = expr();
        expect(')');
        return node;
    }
    Token *tk;
    if (consume_string(&tk)) {
        CVar *var = add_CStr(tk->str, tk->len);
        Node *nd = (Node *)new_ConstNode(var);
        return nd;
    }
    if (consume_ident(&tk)) {
        // TODO:
        // 変数呼び出しか関数呼び出しかは引数を伴うかでのみ判別できる。ここでは同一視すべき
        return NULL;
    }
    return constant();
}
Node *constant() {
    Token *tk;
    if (consume_integer(&tk)) return (Node *)new_NumNode(tk->val);
    if (consume_float(&tk)) return (Node *)new_FloatNode(tk->val);
    if (consume_char(&tk)) return (Node *)new_CharNode(*(tk->str));
    if (consume_enum(&tk)) return (Node *)new_EnumNode(tk->str, tk->len);

    error_here(true, "不明なトークンです。");
}

Node *expression() {
    BlockNode *bnd = new_BlockNode(ND_SET);

    Node anker;
    anker.next = NULL;
    Node *top = &anker;
    do {
        top->next = assignment_expr();
        top = top->next;
    } while (consume(","));
    bnd->block = anker.next;
    return (Node *)bnd;
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
    if (consume("%%="))
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
        expect('(');
        Node *init = check(";") ? nullNode : expression();
        expect(';');
        Node *cond = check(";") ? nullNode : expression();
        expect(';');
        Node *update = check(")") ? nullNode : expression();
        expect(')');
        return (Node *)new_ForNode(init, cond, update, statement());
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