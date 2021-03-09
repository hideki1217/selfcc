#include <stdio.h>
#include <string.h>

#include "selfcc.h"
#include "utility.h"

#define BUFFERSIZE 256

// 便宜用のbuffer
char buffer[BUFFERSIZE];

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
    
    ////////////////////////////グローバル変数たち
    if(fromfile)
        user_input = read_file(filepath);
    filename = fromfile?
        path_filename(filepath)
        :"";
    locals = cc_map_for_var_new();
    globals = cc_avltree_new();
    global_list = cc_vector_new();
    externs = cc_avltree_new();
    tkstream = tokenize(user_input);
    nowToken=tkstream;
    constants = cc_vector_new();

    code=NULL;
    
    Initialize_type_tree();

    nullNode = (Node *)new_NumNode(1);
    /////////////////////////////////////////////
    //program();
    code = translation_unit();

    printf(".Intel_syntax noprefix\n");

    // globalな変数を宣言
    printf(".globl ");
    for(CC_VecNode *nd= global_list->first;nd;nd=nd->next){
        Var *var = (Var*)nd->item;
        string_limitedcopy(buffer,var->name,var->len);
        printf("%s ",buffer);
    } 
    printf("\n");


    // データセクション
    printf("    .section .data\n");
    for(CC_VecNode *nd=constants->first;
        nd!=NULL;
        nd=nd->next)
    {
        CStr *var=(CStr*) nd->item;
        string_limitedcopy(buffer,var->text,var->len);
        printf(".LC%d:\n",var->base.LC_id); 
        printf("    .string \"%s\"\n",buffer);
    }

    // テキストセクション
    printf("    .section .text\n");
    for (Node *now = code; now; now = now->next) {
        type_assign(now);
        gen(now,false);
    }
    return 0;
}
