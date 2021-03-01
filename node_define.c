#include"selfcc.h"

#include<stdlib.h>

void set_Node(Node *node, NodeKind kind) { node->kind = kind; }
void set_BinaryNode(BinaryNode *node, NodeKind kind, Node *lhs, Node *rhs) {
    node->base.kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
}
void set_UnaryNode(UnaryNode *node, NodeKind kind, Node *target) {
    node->base.kind = kind;
    node->target = target;
}
void set_NumNode(NumNode *node, int val) {
    node->base.kind = ND_INT;
    node->val = val;
    node->base.type = find_type_from_name("int");
}
void set_FloatNode(FloatNode *node, float val) {
    node->val = val;
    node->base.kind = ND_FLOAT;
    node->base.type = find_type_from_name("float");
}
void set_CharNode(CharNode *node, char val) {
    node->base.type = find_type_from_name("char");
    node->base.kind = ND_CHAR;
    node->val = val;
}
void set_EnumNode(EnumNode *node, char *name, int len) {
    node->name = name;
    node->len = len;
    node->base.kind = ND_ENUM;
    node->base.type->kind = TY_ENUM;
}
void set_ConstNode(ConstNode *nd, CVar *var) {
    nd->const_id = var->LC_id;
    nd->base.kind = ND_STR;
    nd->base.type = var->base.type;
}
void set_CastNode(CastNode *node, Type *cast, Node *target) {
    node->cast = cast;
    node->target = target;
    node->base.kind = ND_CAST;
}
void set_CondNode(CondNode *node, NodeKind kind, Node *cond, Node *T, Node *F) {
    node->base.kind = kind;
    node->cond = cond;
    node->T = T;
    node->F = F;
}
void set_ForNode(ForNode *node, Node *init, Node *cond, Node *update, Node *T) {
    node->base.kind = ND_FOR;
    node->init = init;
    node->cond = cond;
    node->update = update;
    node->T = T;
}
void set_FuncNode(FuncNode *node,Var *var) {
    node->base.kind = ND_FUNCTION;
    node->base.type = var->type;
    node->funcname = var->name;
    node->namelen = var->len;
}
void set_VarNode(VarNode *node, Var *var) {
    if (var->kind == LOCAL)
        node->base.kind = ND_LVAR;
    else
        node->base.kind = ND_GVAR;
    node->var = var;
}
void set_RootineNode(RootineNode *node, char *name, int len, char *moldname,
                     int moldlen) {
    node->base.kind = ND_ROOTINE;
    node->name = name;
    node->namelen = len;
    node->moldname = moldname;
    node->moldlen = moldlen;
}
void set_BlockNode(BlockNode *node, NodeKind kind) { node->base.kind = kind; }
void set_VarInitNode(VarInitNode *node, Var *var, Node *value) {
    if (var->kind == LOCAL)
        node->base.kind = ND_LVARINIT;
    else
        node->base.kind = ND_GVARINIT;
    node->var = var;
    node->value = value;
}
void set_LabelNode(LabelNode *node, NodeKind kind, char *label, int len) {
    node->base.kind = kind;
    node->label = label;
    node->len = len;
}

Node *new_Node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    set_Node(node, kind);
    return node;
}
BinaryNode *new_BinaryNode(NodeKind kind, Node *lhs, Node *rhs) {
    BinaryNode *node = calloc(1, sizeof(BinaryNode));
    set_BinaryNode(node, kind, lhs, rhs);
    return node;
}
UnaryNode *new_UnaryNode(NodeKind kind, Node *target) {
    UnaryNode *node = calloc(1, sizeof(UnaryNode));
    set_UnaryNode(node, kind, target);
    return node;
}
NumNode *new_NumNode(int val) {
    NumNode *node = calloc(1, sizeof(NumNode));
    set_NumNode(node, val);
    return node;
}
FloatNode *new_FloatNode(float val) {
    FloatNode *node = calloc(1, sizeof(FloatNode));
    set_FloatNode(node, val);
    return node;
}
CharNode *new_CharNode(char val) {
    CharNode *node = calloc(1, sizeof(CharNode));
    set_CharNode(node, val);
    return node;
}
EnumNode *new_EnumNode(char *name, int len) {
    EnumNode *node = calloc(1, sizeof(EnumNode));
    set_EnumNode(node, name, len);
    return node;
}
ConstNode *new_ConstNode(CVar *var) {
    ConstNode *nd = calloc(1, sizeof(ConstNode));
    set_ConstNode(nd, var);
    return nd;
}
CastNode *new_CastNode(Type *cast, Node *target) {
    CastNode *nd = calloc(1, sizeof(CastNode));
    set_CastNode(nd, cast, target);
    return nd;
}
CondNode *new_CondNode(NodeKind kind, Node *cond, Node *T, Node *F) {
    CondNode *node = calloc(1, sizeof(CondNode));
    set_CondNode(node, kind, cond, T, F);
    return node;
}
ForNode *new_ForNode(Node *init, Node *cond, Node *update, Node *T) {
    ForNode *node = calloc(1, sizeof(ForNode));
    set_ForNode(node, init, cond, update, T);
    return node;
}
FuncNode *new_FuncNode(Var *var) {
    FuncNode *node = calloc(1, sizeof(FuncNode));
    set_FuncNode(node, var);
    return node;
}
VarNode *new_VarNode(Var *var) {
    VarNode *node = calloc(1, sizeof(VarNode));
    set_VarNode(node, var);
    return node;
}
RootineNode *new_RootineNode(char *name, int len, char *moldname, int moldlen) {
    RootineNode *node = calloc(1, sizeof(RootineNode));
    set_RootineNode(node, name, len, moldname, moldlen);
    return node;
}
BlockNode *new_BlockNode(NodeKind kind) {
    BlockNode *node = calloc(1, sizeof(BlockNode));
    set_BlockNode(node, kind);
    return node;
}
VarInitNode *new_VarInitNode(Var *var, Node *value) {
    VarInitNode *node = calloc(1, sizeof(VarInitNode));
    set_VarInitNode(node, var, value);
    return node;
}
LabelNode *new_LabelNode(NodeKind kind, char *label, int len) {
    LabelNode *node = calloc(1, sizeof(LabelNode));
    set_LabelNode(node, kind, label, len);
    return node;
}