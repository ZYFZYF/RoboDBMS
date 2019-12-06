%{
#include<cstdio>
#include<iostream>
#include "../Attr.h"
#include "bison.h"
%}

%%
[0-9]+                  yylval.integer = atoi(yytext); return INTEGER;
[+-]?[0-9]+\.[0-9]*     yylval.real = atof(yytext); return REAL;
[a-zA-Z][a-zA-Z0-9_]*   yylval.str = strdup(yytext); return IDENTIFIER;
'[^']*'                 yylval.str = strdup(yytext); return STR;
"=="                    yylval.comparator = EQ_OP; return OP;
"!="                    yylval.comparator = NE_OP; return OP;
"<>"                    yylval.comparator = NE_OP; return OP;
"<"                     yylval.comparator = LT_OP; return OP;
"<="                    yylval.comparator = LE_OP; return OP;
">"                     yylval.comparator = GT_OP; return OP;
">="                    yylval.comparator = GE_OP; return OP;
%%