%{
#include <stdio.h>
#include "y.tab.h"

int
yywrap(void){
    return 1;
}
%}
%%
"+" return TK_ADD;
"-" return TK_SUB;
"*" return TK_MUL;
"/" return TK_DIV;
[1-9][0-9]* {
    double temp;
    sscanf(yytext, "%lf", &temp);
    yylval.int_value = temp;
    return TY_INT;
}
[0-9]*\.[0-9]* {
    double temp;
    sscanf(yytext, "%lf", &temp);
    yylval.float_value = temp;
    return TY_FLOAT;
}
\"*\" {
    char *temp;
    int len;
    
}

%%