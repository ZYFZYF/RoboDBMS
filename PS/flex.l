%{
#include<cstdio>
#include<iostream>
#include<vector>
#include "../Attr.h"
#include "../PS/PS_Node.h"
#include "../PS/PS_ShowDatabases.h"
#include "../SM/SM_Manager.h"
#include "bison.h"
%}

%%
[0-9]+                  yylval.str = strdup(yytext); return STR_INTEGER;
[+-]?[0-9]+\.[0-9]*     yylval.str = strdup(yytext); return STR_REAL;

SHOW|show               return SHOW;
DESC|desc               return DESC;
USE|use                 return USE;
CREATE|create           return CREATE;
ADD|add                 return ADD;
DROP|drop               return DROP;
UPDATE|update           return UPDATE;
INSERT|insert           return INSERT;
ALTER|alter             return ALTER;
SELECT|select           return SELECT;
QUIT|quit               return QUIT;

DATABASES|databases     return DATABASES;
DATABASE|database       return DATABASE;
TABLES|tables           return TABLES;
TABLE|table             return TABLE;
INDEX|index             return INDEX;
PRIMARY|primary         return PRIMARY;
KEY|key                 return KEY;
DEFAULT|default         return DEFAULT;
REFERENCES|references   return REFERENCES;
FOREIGN|foreign         return FOREIGN;
CONSTRAINT|constraint   return CONSTRAINT;

ON|on                   return P_ON;
SET|set                 return P_SET;
WHERE|where             return P_WHERE;
INTO|into               return P_INTO;
NOT|not                 return P_NOT;
NULL|null               return P_NULL;
VALUES|values           return P_VALUES;
FROM|from               return P_FROM;

INT|int                 return T_INT;
BIGINT|bigint           return T_BIGINT;
CHAR|char               return T_CHAR;
CHARACTER|character     return T_CHAR;
VARCHAR|varchar         return T_VARCHAR;
DATE|date               return T_DATE;
DECIMAL|decimal         return T_DECIMAL;

[a-zA-Z][a-zA-Z0-9_]*   yylval.str = strdup(yytext); return IDENTIFIER;
'[^']*'                 {yylval.str = new char[strlen(yytext)-1];memcpy(yylval.str,yytext+1,strlen(yytext)-2);yylval.str[strlen(yytext)-2]='\0';return STR;}


"=="                    yylval.comparator = EQ_OP; return OP;
"!="                    yylval.comparator = NE_OP; return OP;
"<>"                    yylval.comparator = NE_OP; return OP;
"<"                     yylval.comparator = LT_OP; return OP;
"<="                    yylval.comparator = LE_OP; return OP;
">"                     yylval.comparator = GT_OP; return OP;
">="                    yylval.comparator = GE_OP; return OP;

[*/+\-=<>':;,.|&()]  {return yytext[0];}

%%