#include"utility.h"

#include<string.h>

int max(int x,int y){
    return x>y? x:y;
}
int min(int x,int y){
    return x<y? x:y;
}

void swap(void **a,void **b){
    void * temp=*a;
    *a=*b;
    *b=temp;
}

void string_limitedcopy(char *s, char *str, int len) {
    strncpy(s, str, len);
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