#pragma once

char *read_file(char *path);

int max(int x,int y);
int min(int x,int y);

void swap(void **a,void **b);

/*sにstrをlen文字分コピー*/
void string_limitedcopy(char *s, char *str, int len);

int compare_string(const char *a, int a_len, const char *b, int b_len);