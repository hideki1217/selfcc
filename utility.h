#pragma once

#define times(n) for(int i=0;i<(int)n;i++)

#define ISNULL(a) ((a) == NULL)
#define ISNNULL(a) ((a) != NULL)

#define PAIR_STR_LEN(str) str,strlen(str)

#define BUFFER_SIZE 256

extern char buffer[BUFFER_SIZE];

typedef struct String String;
struct String{
    char *str;
    int len;
};
int string_cmp(const char *a, int a_len, const char *b, int b_len);
/*sにstrをlen文字分コピー*/
void string_ncopy(char s[], char *str, int n);
void string_print(char *s,int n);

typedef struct Integer Integer;
struct Integer{
    int val;
};
Integer *new_Integer(int val);

typedef union Object Object;
union Object{
    void *ptr;
    char cval;
    int ival;
    long lval;
    String string;
};
#define obj_ptr(obj) (obj).ptr
#define obj_char(obj) (obj).cval
#define obj_int(obj) (obj).ival
#define obj_long(obj) (obj).lval
#define obj_str(obj) (obj).string.str
#define obj_strlen(obj) (obj).string.len

int max(int x,int y);
int min(int x,int y);

void swap(void **a,void **b);

int is_alnum(char c);
int is_alp(char c);

/**
 * @brief  エラーメッセージを表示して停止する関数
 * @note   
 * @param  *msg: エラーメッセージ
 * @retval None
 */
void error(char *msg, ...);