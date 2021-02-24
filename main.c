#include <stdio.h>

#include "selfcc.h"
#include "utility.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が不正です\n");
        return 1;
    }
    // Grobal変数に値をセット
    user_input = argv[1];// read_file(argv[1]);
    filename = "";// path_filename(argv[1]);
    locals = cc_map_for_var_new();
    globals = cc_avltree_new();
    tkstream = tokenize(user_input);
    constants = cc_vector_new();

    type_tree = cc_avltree_new();
    Initialize_type_tree();

    nullNode = (Node *)new_NumNode(1);

    program();

    printf(".Intel_syntax noprefix\n");
    printf(".globl main\n");  // TODO:ここの意味を把握

    for(CC_VecNode *nd=constants->first;
        nd!=NULL;
        nd=nd->next)
    {
        CStr *var=(CStr*) nd->item;
        char s[var->len+1];
        string_limitedcopy(s,var->text,var->len);
        printf(".LC%d:\n",var->base.LC_id);
        printf("    .string \"%s\"\n",s);
    }

    for (Node *now = code; now; now = now->next) {
        type_assign(now);
        gen(now);
    }
    return 0;
}
