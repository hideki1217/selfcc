#include "file.h"

#include <stdbool.h>
#include <string.h>

#include "utility.h"


bool file_exist(char *fpath, int len){
    char path[len+1];
    strncpy(path,fpath,len);
    path[len] = 0;
    // ファイルを開く
    FILE *fp = fopen(path, "r");
    if(fp == NULL)return false;
    fclose(fp);
    return true;
}

char *file_read2str(char *fpath, int len){
    char path[len+1];
    strncpy(path,fpath,len);
    path[len] = 0;
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