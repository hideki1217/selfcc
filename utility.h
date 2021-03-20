#pragma once

typedef struct String String;
struct String{
    char *str;
    int len;
};
int string_cmp(const char *a, int a_len, const char *b, int b_len);
/*sにstrをlen文字分コピー*/
void string_ncopy(char s[], char *str, int n);

typedef union Object Object;
union Object{
    void *ptr;
    char cval;
    int ival;
    long lval;
    String string;
};

char *read_file(char *path);

int max(int x,int y);
int min(int x,int y);

void swap(void **a,void **b);

char *path_filename(char *path);

int is_alnum(char c);
int is_alp(char c);