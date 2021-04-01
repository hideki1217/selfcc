#include "path.h"

#include <string.h>
#include <stdlib.h>

#define is_slash(char) (char) == '/' || (char) == '\\'

void _path_combine(char *buffer,char *r,int rn,char *l,int ln){
    buffer[0] = '\0';

    while(rn>0 && is_slash(r[rn-1]))rn--;
    strncpy(buffer,r,rn);

    buffer[rn] = '/';
    buffer[rn+1] = '\0';

    while(ln>0 && is_slash(*l)){
        l++,ln--;
    }
    strncat(buffer,l,ln);

    buffer[rn+1+ln] = '\0';
}

void _path_filename(char *buffer,char *str,int n){
    buffer[0] = '\0';

    if(is_slash(str[n-1]))return;

    char *pos = str;
    int len = n;
    for(int i= 0;i < n;i++ ){
        if(is_slash(str[i]))pos = str+i+1,len = n-i-1; 
    }

    strncpy(buffer,pos,len);
    buffer[len] = '\0';
}

void _path_dirname(char *buffer,char *str,int n){
    buffer[0] = '\0';

    int len = n;
    for(int i = n-1;i>=0 ; i--){
        if(is_slash(str[i])){
            len = i;
            break;
        }
    }

    strncpy(buffer,str,len);
    buffer[len] = '\0';
}

char *path_combine(char *r,int rn,char *l,int ln){
    char buffer[256];
    _path_combine(buffer,r,rn,l,ln);
    char *res = malloc(strlen(buffer)+1);
    strncpy(res,buffer,strlen(buffer)+1);
    return res;
}

char* path_filename(char *str,int n){
    char buffer[256];
    _path_filename(buffer,str,n);
    char *res = malloc(strlen(buffer)+1);
    strncpy(res,buffer,strlen(buffer)+1);
    return res;
}

char* path_dirname(char *str,int n){
    char buffer[256];
    _path_dirname(buffer,str,n);
    char *res = malloc(strlen(buffer)+1);
    strncpy(res,buffer,strlen(buffer)+1);
    return res;
}

