#include "utility.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "selfcc.h"

// 指定されたファイルの内容を返す
char *read_file(char *path) {
    // ファイルを開く
    FILE *fp = fopen(path, "r");
    if (!fp) error("cannot open %s: %s", path, strerror(errno));

    // ファイルの長さを調べる
    if (fseek(fp, 0, SEEK_END) == -1)
        error("%s: fseek: %s", path, strerror(errno));
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1)
        error("%s: fseek: %s", path, strerror(errno));

    // ファイル内容を読み込む
    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fp);

    // ファイルが必ず"\n\0"で終わっているようにする
    if (size == 0 || buf[size - 1] != '\n') buf[size++] = '\n';
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

// パスからファイル名を取り出す
char *path_filename(char *path) {
    char *p = path, *q = p;
    while (*p != '\0') {
        if (*p == '\\' || *p == '/') q = p + 1;
        p++;
    }
    char *name = calloc(1, strlen(q) + 1);
    strncpy(name, q, strlen(q) + 1);
    return name;
}

int max(int x, int y) { return x > y ? x : y; }
int min(int x, int y) { return x < y ? x : y; }

void swap(void **a, void **b) {
    void *temp = *a;
    *a = *b;
    *b = temp;
}
void string_limitedcopy(char *s, char *text, int len) {
    strncpy(s, text, len);
    s[len] = '\0';
}
// a>b -> 負数, a<b -> 正数, a=b -> 0
int compare_string(const char *a, int a_len, const char *b, int b_len) {
    int a_i = 0, b_i = 0;
    a_len--;
    b_len--;
    while (a[a_i] == b[b_i]) {
        if (a_i == a_len && b_i < b_len) return -1;
        if (a_i < a_len && b_i == b_len) return 1;
        if (a_i == a_len && b_i == b_len) return 0;
        a_i++;
        b_i++;
    }
    return (a[a_i] - b[b_i]);
}