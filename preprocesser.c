#include "preprocesser.h"

/**
 * @brief  アルゴリズムはhttps://github.com/rui314/8cc/wiki/cpp.algo.pdf による
 * @note
 * @retval None
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "collections.h"
#include "file.h"
#include "path.h"
#include "selfcc.h"
#include "utility.h"
#include "vector.h"

static CC_AVLTree *defined_macros /*<Macro *>*/;
static CC_Vector *include_paths /*<char *>*/;
static CC_AVLTree *included_files /*<NULL>*/;

void Initialize_preprocesser() {
    defined_macros = cc_avltree_new();
    include_paths = cc_vector_new();

    cc_vector_pbStr(include_paths, PAIR_STR_LEN("/usr/include"));
    cc_vector_pbStr(include_paths,
                    PAIR_STR_LEN("/usr/lib/gcc/x86_64-linux-gnu/9/include"));
    cc_vector_pbStr(include_paths,
                    PAIR_STR_LEN("/usr/include/x86_64-linux-gnu"));
}
void add_includepath(char *dirpath, int len) {
    char *path = malloc(len + 1);
    strncpy(path, dirpath, len);
    path[len] = 0;
    cc_vector_pbStr(include_paths, path, len);
}

Macro *new_Macro() { return calloc(1, sizeof(Macro)); }
void set_Macro(Macro *macro, char *key, int keylen, Token *begin, Token *end,
               CC_Vector *params) {
    macro->kind = (params) ? MC_FUNC : MC_OBJECT;
    macro->key = key;
    macro->keylen = keylen;
    macro->ts = create_TkSq(begin, end);
    macro->params = params;
}
#define macro_clonets(macro) tksq_clone(macro->ts)

Macro *macro_Search(char *key, int len) {
    return cc_avltree_Search(defined_macros, key, len);
}
void macro_Delete(char *key, int len) {
    Macro *macro = cc_avltree_Search(defined_macros, key, len);
    if (macro) {
        if (macro->params) cc_vector_delete(macro->params);
        Token *iter = macro->ts->begin;
        while (iter != macro->ts->end) {
            Token *tmp = token_next(iter);
            free(iter);
            iter = tmp;
        }
        cc_avltree_DeleteNode(defined_macros, key, len);
    }
}

#define match(token, string)          \
    (token)->len == strlen(string) && \
        memcmp(string, (token)->str, (token)->len) == 0

static Macro *regist_macro(char *key, int keylen) {
    Macro *macro = calloc(1, sizeof(Macro));
    macro->key = key;
    macro->keylen = keylen;
    cc_avltree_Add(defined_macros, macro->key, macro->keylen, macro);
    return macro;
}

/**
 * @brief  TK_MACROENDのノードを返す。
 * @note
 * @param  *start: 探索の始点
 * @retval TK_MACROENDのノードへのpointer
 */
Token *skip2MacroEnd(Token *begin) {
    Token *top = begin;
    for (; top->kind != TK_MACROEND && top->kind != TK_END; top = top->next)
        ;
    return top;
}
Token *skip2EndIf(Token *begin) {
    Token *top = begin;
    int count = 0;
    for (; top->kind != TK_END && (count != 0 || top->kind != TK_MACROENDIF);
         top = top->next) {
        if (top->kind == TK_MACROIF) count++;
        if (top->kind == TK_MACROENDIF) count--;
    }
    return top;
}
#define INCREMENT_ROOT_WITH_SAVE() noneMacro = root, root = root->next
#define INCREMENT_ROOT() root = root->next
#define CONNECT_PRE_2_ROOT()     \
    INCREMENT_ROOT();            \
    token_join(noneMacro, root); \
    root = noneMacro

/** TK_STRINGを連結 beginをTK_STRING出ないとこまで進める*/
static Token *_combine_strings(Token **begin) {
    Token *end = *begin;

    // 長さを計算
    int str_length = 0;
    while (end->kind == TK_STRING) {
        str_length += end->len;
        end = token_next(end);
    }

    // TK_STRING系列が一つだけならそのまま返す
    if (token_next(*begin) == end) {
        Token *tmp = *begin;
        *begin = end;
        return tmp;
    }

    // 文字列を連結してTK_STRING系列の先頭にセット
    char *combined = calloc(str_length + 1, sizeof(char));
    for (Token *x = *begin; x != end; x = token_next(x)) {
        strncat(combined, x->str, x->len);
    }
    (*begin)->str = combined;
    (*begin)->len = str_length;

    //不要なトークンを開放
    Token *frees = token_next(*begin);
    while (frees != end) {
        Token *tmp = frees;
        frees = token_next(frees);
        free(tmp);
    }
    // TK_STRINGじゃないとこまで進める
    Token *tmp = *begin;
    *begin = end;
    return tmp;
}
TkSequence *combine_strings(TkSequence *ts) {
    Token *root = ts->begin;
    Token anker, *noneString = &anker;
    anker.next = root;
    while (root->kind != TK_END) {
        if (root->kind == TK_STRING) {
            noneString = token_prev(root);
            Token *combined = _combine_strings(&root);

            token_join(noneString, combined);
            token_join(combined, root);
        }
        root = token_next(root);
    }
    ts->begin = anker.next;
    ts->end = root;
    return ts;
}

void free_Hideset(TkSequence *ts) {
    Token *root = ts->begin;
    while (root->kind != TK_END) {
        if (root->hs != NULL) {
            hs_delete(root->hs);
            root->hs = NULL;
        }
        root = token_next(root);
    }
    if (root->hs != NULL) {
        hs_delete(root->hs);
        root->hs = NULL;
    }
}
static TkSequence *_expand_include(Token *token, char *basedir, int len);
static bool evaluate_if(Token *root);
/** token[0] * insert * token[1]...*/
#define INSERT(token, insert)          \
    token_join(insert, (token)->next); \
    token_join(token, insert);
#define REMOVE_MACROSECTION(macro_token, noneMacro_token) \
    macro_token = skip2MacroEnd(macro_token);             \
    macro_token = (macro_token)->next;                    \
    token_join(noneMacro_token, macro_token);

static char *CurrentDir;
TkSequence *expand(TkSequence *ts) {
    if (!ts) return NULL;
    Token *root = ts->begin;

    enum { MM_IGNORE, MM_NORMAL } macromode = MM_NORMAL;
    int if_count = 0;

    Token anker, *noneMacro = &anker, *mem;
    anker.next = root;
    while (root->kind != TK_END) {
        if (macromode == MM_IGNORE) {
            if (mem = _consume("elif", &root)) {  // TODO
                if (if_count <= 0) error_at(mem->str, "ifがありません");
                bool res = evaluate_if(root);
                REMOVE_MACROSECTION(root, noneMacro)
                if (!res) continue;
                macromode = MM_NORMAL;
                continue;
            }
            if (_consume("else", &root)) {  // TODO
                if (if_count <= 0) error_at(mem->str, "ifがありません");
                REMOVE_MACROSECTION(root, noneMacro)
                macromode = MM_NORMAL;
                continue;
            }
            if (_consume("endif", &root)) {  // TODO
                if (if_count <= 0) error_at(mem->str, "ifがありません");
                REMOVE_MACROSECTION(root, noneMacro)
                macromode = MM_NORMAL;
                if_count--;
                continue;
            }
            INCREMENT_ROOT();
            continue;
        }
        switch (root->kind) {
            case TK_MACROSTART: {  // マクロ定義部
                INCREMENT_ROOT();
                if (root->kind == TK_MACROEND) {
                    CONNECT_PRE_2_ROOT();
                    continue;
                }
                if (_consume("define", &root)) {
                    // 識別子を確保
                    Token *ident = _expect_ident(&root);
                    Macro *macro = regist_macro(ident->str, ident->len);
                    //関数型マクロなら引数が存在
                    CC_Vector *vec = NULL;
                    if (_consume("(", &root)) {
                        vec = cc_vector_new();
                        while (!_consume(")", &root)) {
                            _consume(",", &root);
                            Token *param = _expect_ident(&root);
                            cc_vector_pbStr(vec, param->str, param->len);
                        }
                    }
                    // マクロの内容
                    Token *begin = root;
                    root = skip2MacroEnd(begin);

                    set_Macro(macro, ident->str, ident->len, begin, root, vec);

                    CONNECT_PRE_2_ROOT();
                    continue;
                }
                if (_consume("undef", &root)) {
                    Token *ident = _expect_ident(&root);
                    macro_Delete(ident->str, ident->len);
                    root = skip2MacroEnd(ident);

                    CONNECT_PRE_2_ROOT();
                    continue;
                }
                if (_consume("include", &root)) {
                    char *dir = CurrentDir;
                    TkSequence *ex = _expand_include(root, PAIR_STR_LEN(dir));
                    root = skip2MacroEnd(root);
                    INCREMENT_ROOT();  // MACROEND

                    token_join(noneMacro, ex->begin);
                    token_join(ex->end->prev, root);

                    root = noneMacro;
                    continue;
                }
                if (_consume("if", &root)) {  // TODO
                    if_count++;
                    bool res = evaluate_if(root);
                    REMOVE_MACROSECTION(root, noneMacro)
                    if (res) continue;
                    macromode = MM_IGNORE;
                    continue;
                }
                if (_consume("elif", &root) ||
                    _consume("else", &root)) {  // TODO
                    if (if_count <= 0) error_at(mem->str, "ifがありません");
                    root = skip2EndIf(root);
                    REMOVE_MACROSECTION(root, noneMacro)
                    if_count--;
                    continue;
                }
                if (_consume("endif", &root)) {  // TODO
                    if (if_count <= 0) error_at(mem->str, "ifがありません");
                    REMOVE_MACROSECTION(root, noneMacro)
                    if_count--;
                    continue;
                }
                if (_consume("ifdef", &root)) {  // TODO
                    if_count++;
                    Token *ident = _consume_ident(&root);
                    REMOVE_MACROSECTION(root, noneMacro)
                    if (ISNNULL(ident) &&
                        ISNNULL(
                            macro_Search(ident->str, ident->len)))  // trueの時
                        continue;

                    macromode = MM_IGNORE;
                    continue;
                }
                if (_consume("ifndef", &root)) {  // TODO
                    if_count++;
                    Token *ident = _consume_ident(&root);
                    REMOVE_MACROSECTION(root, noneMacro)
                    if (ISNNULL(ident) &&
                        ISNNULL(
                            macro_Search(ident->str, ident->len)))  // falseの時
                    {
                        macromode = MM_IGNORE;
                        continue;
                    }

                    continue;
                }
            }
            case TK_IDENT: {  // マクロ展開部
                Token *ident = root;
                Macro *macro =
                    cc_avltree_Search(defined_macros, ident->str, ident->len);
                if (macro == NULL) break;
                if (hs_find(token_geths(ident), ident->str, ident->len))
                    break;  // HideSetに入っていたら何もしない

                INCREMENT_ROOT();  // ident
                if (macro->kind == MC_FUNC) {
                    CC_Vector *args = cc_vector_new();
                    Token *begin, *end;
                    {
                        int bracket_n = 0;
                        _expect("(", &root);
                        if (!(end = _consume(")", &root))) {  // 引数0と1を区別
                            begin = root;
                            while (1) {
                                if (end = _consume(")", &root)) {
                                    if (bracket_n == 0) {
                                        TkSequence *ts =
                                            (begin == end)
                                                ? NULL
                                                : create_TkSq(begin, end);
                                        cc_vector_pbPtr(args, ts);
                                        break;
                                    }
                                    bracket_n--;
                                    continue;
                                }
                                if (end = _consume(",", &root)) {
                                    if (bracket_n == 0) {
                                        TkSequence *ts =
                                            (begin == end)
                                                ? NULL
                                                : create_TkSq(begin, end);
                                        cc_vector_pbPtr(args, ts);
                                        begin = root;
                                    }
                                    continue;
                                }
                                if (_consume("(", &root)) bracket_n++;
                                INCREMENT_ROOT();
                            }
                        }
                    }
                    if (args->size != macro->params->size)
                        error_at(ident->str, "引数の個数が違います。");

                    TkSequence *TS = macro_clonets(macro);
                    HideSet *HS =
                        hs_cross(token_geths(ident), token_geths(end));
                    hs_add(HS, ident->str, ident->len);
                    TkSequence *res = subst(TS, macro->params, args, HS);

                    // noneMacroとrootの間にマクロ呼び出しTokenがゴミになっている
                    token_join(noneMacro, root);  // TODO: [ゴミ集め]

                    tksq_insert(ts, root, res);

                    root = noneMacro;
                } else if (macro->kind == MC_OBJECT) {
                    TkSequence *TS = macro_clonets(macro);
                    HideSet *HS = hs_clone(token_geths(ident));
                    hs_add(HS, ident->str, ident->len);
                    TkSequence *res = subst(TS, NULL, NULL, HS);

                    // noneMacroとrootの間にマクロ呼び出しTokenがゴミになっている
                    token_join(noneMacro, root);  // TODO: [ゴミ集め]

                    tksq_insert(ts, root, res);

                    root = noneMacro;
                }
                INCREMENT_ROOT_WITH_SAVE();
                continue;
            }
        }
        INCREMENT_ROOT_WITH_SAVE();
    }
    ts->begin = anker.next;
    ts->end = root;
    return ts;
}
TkSequence *subst(TkSequence *is, CC_Vector *fp, CC_Vector *ap, HideSet *hs) {
    Token anker, *root = is->begin;
    anker.str = "";
    anker.len = 0;
    token_join(&anker, root);

    Token *ident, *OS = &anker;
    int index;
    while (root->kind != TK_END) {
        if (fp != NULL && _check("#", root) &&
            (ident = _check_ident(token_next(root)))) {
            // 引数リストに存在するか?
            index = cc_vector_findStr(fp, ident->str, ident->len);
            if (index >= 0) {
                INCREMENT_ROOT();  // "#"
                INCREMENT_ROOT();  // ident
                Token *str_token = stringize(select(index, ap));
                INSERT(OS, str_token);
                OS = str_token;

                continue;
            }
        }
        if (_consume("##", &root)) {
            if (fp) {
                index = cc_vector_findStr(fp, root->str, root->len);
                if (index >= 0) {  // 置換対象
                    TkSequence *actual = tksq_clone(select(index, ap));
                    if (actual == NULL) {
                        token_join(OS, root);  // TODO: [ゴミ集め]
                        continue;
                    }
                    Token *cbd = glue(OS, actual->begin), *gbg = OS;
                    token_join(OS->prev, cbd);
                    token_join(cbd, actual->begin->next);

                    free(gbg);
                    free(actual->end);

                    OS = actual->end->prev;
                    INCREMENT_ROOT();
                    continue;
                }
            }
            // 普通のトークン
            Token *cbd = glue(OS, root);
            token_join(OS->prev, cbd);
            token_join(cbd, root->next);

            free(OS);
            free(root);

            OS = cbd;
            root = OS->next;
            continue;
        }
        if ((ident = _check_ident(root)) != NULL && fp != NULL &&
            (index = cc_vector_findStr(fp, ident->str, ident->len)) >= 0) {
            INCREMENT_ROOT();  // ident
            TkSequence *actual = tksq_clone(select(index, ap));
            Token *ds;
            if (ds = _check("##", root)) {
                if (actual == NULL) {
                    INCREMENT_ROOT();  // "##"
                    int jndex = cc_vector_findStr(fp, root->str, root->len);
                    if (jndex >= 0) {
                        INCREMENT_ROOT();  // ident
                        actual = tksq_clone(select(jndex, ap));
                        if (actual) {
                            token_join(OS, actual->begin);
                            OS = actual->end->prev;

                            free(actual->end);
                            continue;
                        }
                    }
                    // jndex>= 0 ^ actual == NULL or jndex<0　
                    continue;
                } else {
                    // root = "##"
                    token_join(OS, actual->begin);
                    OS = actual->end->prev;
                    continue;
                }
            }
            TkSequence *ts = expand(actual);
            if (ts) {
                token_join(OS, ts->begin);
                OS = ts->end->prev;
            }
            continue;
        }
        token_join(OS, root);
        OS = root;
        INCREMENT_ROOT();  // some token
    }

    token_join(OS, is->end);
    TOKEN_FOR(token, anker.next) {
        HideSet *token_hs = token_geths(token);
        LIST_FOR(iter, hs) {
            String str = iter->obj.string;
            hs_add(token_hs, str.str, str.len);
        }
    }
    construct_TkSq(is, anker.next, root);
    return is;
}
/**os_endとrtの字面で合成したトークンを生成*/
Token *glue(Token *os_end, Token *rt) {
    int str_length = os_end->len + rt->len;
    char *combined = calloc(str_length + 1, sizeof(char));
    strncat(combined, os_end->str, os_end->len);
    strncat(combined, rt->str, rt->len);

    TkSequence *res = tokenize(combined);
    Token *cbd_tk = res->begin;
    cbd_tk->hs = hs_cross(token_geths(os_end), token_geths(rt));

    free(res);

    return cbd_tk;
}
Token *stringize(const TkSequence *ts) {
    // 長さを計算
    int str_length = 0;
    TOKEN_FOR(token, ts->begin) { str_length += token->len; }

    // TSの要素が一つだけなら文字列にしたものを返す
    if (token_next(ts->begin) == ts->end) {
        return new_Token(TK_STRING, NULL, ts->begin->str, ts->begin->len);
    }

    // 文字列を連結してTSの先頭にセット
    char *combined = calloc(str_length + 1, sizeof(char));
    TOKEN_FOR(token, ts->begin) { strncat(combined, token->str, token->len); }

    return new_Token(TK_STRING, NULL, combined, str_length);
}
static void find_includeitem(char *buffer, Token *token, char *basedir,
                             int len) {
    if (token->kind != TK_STRING && token->kind != TK_INCLUDEPATH)
        error_at(token->str, "パスではありません");

    if (token->kind == TK_STRING) {  // "includepath"
        // nowdirで調べる
        _path_combine(buffer, basedir, len, token->str, token->len);
        if (file_exist(buffer, strlen(buffer))) return;
    }
    // include pathで調べる
    for (int i = 0; i < include_paths->size; i++) {
        String name = include_paths->_[i].string;
        _path_combine(buffer, name.str, name.len, token->str, token->len);
        if (file_exist(buffer, strlen(buffer))) return;
    }

    error_at(token->str, "存在しないファイルやディレクトリです");
}
static TkSequence *_expand_include(Token *token, char *basedir, int len) {
    char buffer[BUFFER_SIZE], dir[BUFFER_SIZE];

    find_includeitem(buffer, token, basedir, len);

    _path_dirname(dir, PAIR_STR_LEN(buffer));

    char *program = file_read2str(PAIR_STR_LEN(buffer));
    TkSequence *ts = tokenize(program);

    CurrentDir = dir;  // 次のベースディレクトリを登録
    TkSequence *expand_program = expand(ts);
    CurrentDir = basedir;  // 元のベースディレクトリに戻ってくる

    return expand_program;
}
static bool evaluate_if(Token *root) {
    return true;  // TODO
}

TkSequence *preproccess(TkSequence *ts, char *dirpath) {
    CurrentDir = dirpath;
    ts = expand(ts);
    free_Hideset(ts);
    ts = combine_strings(ts);
    return ts;
}

#undef INCREMENT_ROOT
#undef INCREMENT_ROOT_WITH_SAVE
#undef match