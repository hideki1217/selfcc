#include "utility.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


char buffer[BUFFER_SIZE];

int max(int x, int y) { return x > y ? x : y; }
int min(int x, int y) { return x < y ? x : y; }

void swap(void **a, void **b) {
    void *temp = *a;
    *a = *b;
    *b = temp;
}
void string_ncopy(char *s, char *text, int len) {
    strncpy(s, text, len);
    s[len] = '\0';
}
// a>b -> 負数, a<b -> 正数, a=b -> 0
/**
 * @brief  辞書順比較:: - => a>b, + =>a<b, 0 => a=b
 * @note   
 * @param  *a: 比較対象1
 * @param  a_len: 比較対象1の長さ
 * @param  *b: 比較対象2
 * @param  b_len: 比較対象2の長さ
 * @retval 比較結果
 */
int string_cmp(const char *a, int a_len, const char *b, int b_len) {
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
void string_print(char *s,int n){
    strncpy(buffer,s,n);
    printf("%s",buffer);
}

Integer *new_Integer(int val){
    Integer *integer = calloc(1,sizeof(Integer));
    integer->val = val;
    return integer;
}


int is_alnum(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') || (c == '_');
}

int is_alp(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

void error(char *msg, ...) {
    va_list ap;
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    exit(1);
}