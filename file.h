#pragma once 

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

/**
 * @brief  ファイルが存在するか検査
 * @note   
 * @param  *fpath: ファイルへのパス
 * @param  len: パスの文字列としての長さ
 * @retval 存在するか
 */
bool file_exist(char *fpath, int len);

/**
 * @brief  fpathからファイルを読みだす。
 * @note   
 * @param  *fpath: ファイルへの絶対パス
 * @param  len: 文字列としての長さ
 * @retval ファイルの中身
 */
char *file_read2str(char *fpath, int len);