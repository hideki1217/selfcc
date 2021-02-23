#include <stdio.h>

#include "selfcc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が不正です\n");
        return 1;
    }
    // Grobal変数に値をセット
    user_input = argv[1];
    locals = cc_map_for_var_new();
    globals = cc_avltree_new();
    tkstream = tokenize(user_input);

    type_tree = cc_avltree_new();
    Initialize_type_tree();

    nullNode = (Node *)new_NumNode(1);

    program();

    printf(".Intel_syntax noprefix\n");
    printf(".globl main\n");  // TODO:ここの意味を把握

    for (Node *now = code; now; now = now->next) {
        type_assign(now);
        gen(now);
    }
    return 0;
}
