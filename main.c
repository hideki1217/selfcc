#include <stdio.h>
#include <string.h>

#include "selfcc.h"
#include "utility.h"

char buffer[BUFFERSIZE];

void code_generate(Node *code);
void debug_pp(){
    const int MAX_WIDTH = 50;
    Token *t = tkstream;
    int count = 0;
    while(t->kind != TK_EOF){
        if((count = count + t->len +1)>MAX_WIDTH){
            printf("\n");
            count = t->len +1;
        }
        string_limitedcopy(buffer,t->str,t->len);
        printf("%s ",buffer);
        t = t->next;
    }
}

int main(int argc, char **argv) {
    bool fromfile = true;
    bool dpp = false;
    char *filepath;
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--row", 5) == 0) {
            user_input = argv[i + 1];
            fromfile = false;
            continue;
        }
        if( strncmp(argv[i],"--dpp",5) == 0){
            dpp = true;
            continue;
        }
        filepath = argv[i];
    }
    ////////////////////////////グローバル変数の初期化
    locals = lvar_manager_new();
    globals = cc_avltree_new();
    global_list = cc_vector_new();
    externs = cc_avltree_new();
    constants = cc_vector_new();
    Initialize_type_tree();
    initialize_parser();
    Initialize_preprocesser();

    nullNode = (Node *)new_NumNode(1);// ここは0以外
    /////////////////////////////////////////////
    if (fromfile) user_input = read_file(filepath);
    filename = fromfile ? path_filename(filepath) : "";
    tkstream = tokenize(user_input);              // トークン化
    nowToken = tkstream = preproccess(tkstream);  // プリプロセス
    if(dpp)debug_pp();
    Node *code = translation_unit();              // 抽象構文木化
    code_generate(code);                          // コード生成
    return 0;
}

void code_generate(Node *code) {
    printf(".Intel_syntax noprefix\n");
    // globalな変数を宣言
    foreach(nd,global_list) {
        Var *var = nd->item.ptr;
        string_limitedcopy(buffer, var->name, var->len);
        printf(".globl %s \n", buffer);
    }
    // データセクション
    printf("    .data\n");
    foreach(nd,constants) {
        CStr *var = nd->item.ptr;
        string_limitedcopy(buffer, var->text, var->len);
        printf(".LC%d:\n", var->base.LC_id);
        printf("    .string \"%s\"\n", buffer);
    }
    // テキストセクション
    printf("    .text\n");
    for (Node *now = code; now; now = now->next) {
        type_assign(now);
        gen(now, false);
    }
}
