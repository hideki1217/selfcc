#include <stdio.h>
#include <string.h>

#include "selfcc.h"
#include "utility.h"

int main(int argc, char **argv) {
    bool fromfile=true;
    char *filepath;
    for(int i=1;i<argc;i++){
        if(strncmp(argv[i],"--row",5)==0){
            user_input=argv[i+1];
            fromfile=false;
        }
        filepath=argv[i];
    }
    
    if(fromfile)
        user_input = read_file(filepath);
    filename = fromfile?
        path_filename(filepath)
        :"";
    locals = cc_map_for_var_new();
    globals = cc_avltree_new();
    tkstream = tokenize(user_input);
    constants = cc_vector_new();

    type_tree = cc_avltree_new();
    Initialize_type_tree();

    nullNode = (Node *)new_NumNode(1);

    program();

    printf(".Intel_syntax noprefix\n");
    printf(".globl main\n");  


    // データセクション
    printf("    .section .data\n");
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

    // テキストセクション
    printf("    .section .text\n");
    for (Node *now = code; now; now = now->next) {
        type_assign(now);
        gen(now,false);
    }
    return 0;
}
