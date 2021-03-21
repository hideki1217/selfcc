#include"selfcc.h"

#include<stdlib.h>

void set_Node(Node *node, NodeKind kind) { node->kind = kind; node->pos = tkstream? tkstream:NULL;}
void set_BinaryNode(BinaryNode *node, NodeKind kind, Node *lhs, Node *rhs) {
    set_Node((Node*)node,kind);
    node->lhs = lhs;
    node->rhs = rhs;
}
void set_UnaryNode(UnaryNode *node, NodeKind kind, Node *target) {
    set_Node((Node*)node,kind);
    node->target = target;
}
void set_NumNode(NumNode *node, int val) {
    set_Node((Node*)node,ND_INT);
    node->val = val;
    node->base.type = find_type_from_name("int");
}
void set_FloatNode(FloatNode *node, float val) {
    set_Node((Node*)node,ND_FLOAT);
    node->val = val;
    node->base.type = find_type_from_name("float");
}
void set_CharNode(CharNode *node, char val) {
    set_Node((Node*)node,ND_CHAR);
    node->base.type = find_type_from_name("char");
    node->val = val;
}
void set_EnumNode(EnumNode *node, char *name, int len) {
    set_Node((Node*)node,ND_ENUM);
    node->name = name;
    node->len = len;
    node->base.type->kind = TY_ENUM;
}
void set_ConstNode(ConstNode *nd, CVar *var) {
    set_Node((Node*)nd,ND_STR);
    nd->const_id = var->LC_id;
    nd->base.type = var->base.type;
}
void set_CastNode(CastNode *node, Type *cast, Node *target) {
    set_Node((Node*)node,ND_CAST);
    node->cast = cast;
    node->target = target;
}
void set_CondNode(CondNode *node, NodeKind kind, Node *cond, Node *T, Node *F) {
    set_Node((Node*)node,kind);
    node->cond = cond;
    node->T = T;
    node->F = F;
}
void set_ForNode(ForNode *node, Node *init, Node *cond, Node *update, Node *T) {
    set_Node((Node*)node,ND_FOR);
    node->init = init;
    node->cond = cond;
    node->update = update;
    node->T = T;
}
void set_CallNode(CallNode *node,Node *ident){
    set_Node((Node*)node,ND_CALL);
    node->func = ident;
}
void set_VarNode(VarNode *node, Var *var) {
    NodeKind kind;
    if (var->kind == LOCAL)
        kind = ND_LVAR;
    else
        kind = ND_GVAR;
    set_Node((Node*)node,kind);
    node->var = var;
    node->base.type = var->type;
}
void set_RootineNode(RootineNode *node,Var *var,VarNode *args,Node *block) {
    set_Node((Node*)node,ND_ROOTINE);
    node->base.type = var->type;
    node->func = var;
    node->block = block;
    node->arg =args;
}
void set_BlockNode(BlockNode *node, NodeKind kind) { set_Node((Node*)node,kind); }
void set_VarInitNode(VarInitNode *node, Var *var, Node *value) {
    NodeKind kind;
    if (var->kind == LOCAL)
        kind = ND_LVARINIT;
    else
        kind = ND_GVARINIT;
    set_Node((Node*)node,kind);
    node->var = var;
    node->value = value;
}
void set_LabelNode(LabelNode *node, NodeKind kind,int index) {
    set_Node((Node*)node,kind);
    node->jumpTo = index;
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
CallNode *new_CallNode(Node *func) {
    CallNode *node = calloc(1, sizeof(CallNode));
    set_CallNode(node, func);
    return node;
}
VarNode *new_VarNode(Var *var) {
    VarNode *node = calloc(1, sizeof(VarNode));
    set_VarNode(node, var);
    return node;
}
RootineNode *new_RootineNode(Var *var,VarNode *args,Node *block) {
    RootineNode *node = calloc(1, sizeof(RootineNode));
    set_RootineNode(node, var,args,block);
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
LabelNode *new_LabelNode(NodeKind kind,int index) {
    LabelNode *node = calloc(1, sizeof(LabelNode));
    set_LabelNode(node, kind, index);
    return node;
}