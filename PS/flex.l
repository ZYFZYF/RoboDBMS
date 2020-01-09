%{
#include<cstdio>
#include<iostream>
#include<vector>
#include "../Attr.h"
#include "../PS/PS_Node.h"
#include "../PS/PS_ShowDatabases.h"
#include "../SM/SM_Manager.h"
#include "../PS/PS_Expr.h"
#include "bison.h"
%}

%%
-?[0-9]+                yylval.str = strdup(yytext); return STR_INTEGER;
[+-]?[0-9]+\.[0-9]*     yylval.str = strdup(yytext); return STR_REAL;

SHOW|show               return SHOW;
USE|use                 return USE;
CREATE|create           return CREATE;
ADD|add                 return ADD;
DROP|drop               return DROP;
UPDATE|update           return UPDATE;
INSERT|insert           return INSERT;
ALTER|alter             return ALTER;
SELECT|select           return SELECT;
DELETE|delete           return DELETE;
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
IS|is                   return P_IS;
AS|as                   return P_AS;
GROUP|group             return P_GROUP;
BY|by                   return P_BY;
ORDER|order             return P_ORDER;
DESC|desc               return P_DESC;
ASC|asc                 return P_ASC;
LIMIT|limit             return P_LIMIT;

INT|int                 return T_INT;
BIGINT|bigint           return T_BIGINT;
CHAR|char               return T_CHAR;
CHARACTER|character     return T_CHAR;
VARCHAR|varchar         return T_VARCHAR;
DATE|date               return T_DATE;
DECIMAL|decimal         return T_DECIMAL;

AND|and                 return C_AND;
OR|or                   return C_OR;
MAX|max                 return C_MAX;
MIN|min                 return C_MIN;
AVG|avg                 return C_AVG;
SUM|sum                 return C_SUM;
COUNT|count             return C_COUNT;

[a-zA-Z][a-zA-Z0-9_]*   yylval.str = strdup(yytext); return IDENTIFIER;
'[^']*'                 {yylval.str = new char[strlen(yytext)-1];memcpy(yylval.str,yytext+1,strlen(yytext)-2);yylval.str[strlen(yytext)-2]='\0';return STR;}

[*/+\-%=<>':;,.|&()]     return yytext[0];

%%