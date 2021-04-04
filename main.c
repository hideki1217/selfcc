#include <stdio.h>
#include <string.h>

#include "file.h"
#include "path.h"
#include "selfcc.h"
#include "utility.h"

void code_generate(Node *code);
#define print_tab(n) \
    times(n) { fprintf(stderr, "\t"); }

void debug_pp(TkSequence *ts) {
    const int MAX_WIDTH = 50;
    Token *t = ts->begin;
    int indent = 0;
    while (t->kind != TK_END) {
        string_ncopy(buffer, t->str, t->len);
        if (t->kind == TK_STRING) {
            fprintf(stderr, "\"%s\" ", buffer);
        } else if (t->kind == TK_CHAR) {
            fprintf(stderr, "\'%s\'", buffer);
        } else if (token_match(t, "{", 1)) {
            fprintf(stderr, "%s \n", buffer);
            indent++;
            print_tab(indent);
        } else if (token_match(t, "}", 1)) {
            fprintf(stderr, "%s \n", buffer);
            indent--;
            print_tab(indent);
        } else if (token_match(t, ";", 1)) {
            fprintf(stderr, "%s \n", buffer);
            print_tab(indent);
        } else {
            fprintf(stderr, "%s ", buffer);
        }
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
        if (strncmp(argv[i], "--dpp", 5) == 0) {
            dpp = true;
            continue;
        }
        filepath = argv[i];
    }
    char *dirpath = path_dirname(PAIR_STR_LEN(filepath));
    ////////////////////////////グローバル変数の初期化
    locals = lvar_manager_new();
    globals = cc_avltree_new();
    global_list = cc_bidlist_new();
    externs = cc_avltree_new();
    constants = cc_bidlist_new();
    initialize_typemgr();
    initialize_parser();
    Initialize_preprocesser();

    nullNode = (Node *)new_NumNode(1);  // ここは0以外
    ////////////////////////////////////////////
    TkSequence *tokens;

    if (fromfile) user_input = file_read2str(filepath, strlen(filepath));
    filename =
        fromfile ? path_filename(filepath, strlen(filepath)) : "stdinput";
    tokens = tokenize(user_input);          // トークン化
    tokens = preproccess(tokens, dirpath);  // プリプロセス
    if (dpp) debug_pp(tokens);        // プリプロセスの結果デバッグ
    tkstream = tokens->begin;         // streamにセット
    Node *code = translation_unit();  // 抽象構文木化
    code_generate(code);              // コード生成
    return 0;
}

void code_generate(Node *code) {
    printf(".Intel_syntax noprefix\n");
    // globalな変数を宣言
    LIST_FOR(nd, global_list) {
        Var *var = nd->obj.ptr;
        string_ncopy(buffer, var->name, var->len);
        printf(".globl %s \n", buffer);
    }
    // データセクション
    printf("    .data\n");
    LIST_FOR(nd, constants) {
        CStr *var = nd->obj.ptr;
        string_ncopy(buffer, var->text, var->len);
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
