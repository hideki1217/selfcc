#pragma once

char* path_combine(char *r,int rn,char *l,int ln);
char* path_filename(char *str,int n);
char* path_dirname(char *str,int n);

/** bufferに連結した結果を書き込む*/
void _path_combine(char *buffer,char *r,int rn,char *l,int ln);
void _path_filename(char *buffer,char *str,int n);
void _path_dirname(char *buffer,char *str,int n);