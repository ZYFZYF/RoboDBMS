%{
#include<cstdio>
#include<iostream>
#include "../Attr.h"
#include "../PS/PS_Node.h"
#include "../PS/PS_ShowDatabases.h"
#include "bison.h"
%}

%%
[0-9]+                  yylval.integer = atoi(yytext); return INTEGER;
[+-]?[0-9]+\.[0-9]*     yylval.real = atof(yytext); return REAL;

SHOW                    return SHOW;
USE                     return USE;
CREATE                  return CREATE;
ADD                     return ADD;
DROP                    return DROP;
UPDATE                  return UPDATE;
INSERT                  return INSERT;
ALTER                   return ALTER;
SELECT                  return SELECT;

DATABASES               return DATABASES;
DATABASE                return DATABASE;
TABLES                  return TABLES;
TABLE                   return TABLE;
INDEX                   return INDEX;

ON                      return ON;
SET                     return SET;
WHERE                   return WHERE;
INTO                    return INTO;

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