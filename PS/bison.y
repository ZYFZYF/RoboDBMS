%{
#include<cstdio>
#include "../Attr.h"
extern int yylex (void);
void yyerror(const char *s, ...);

%}
//让yylval不仅返回int数据
%union{
  int integer;
  float real;
  char * str;
  enum CompOp comparator;
}

//定义标识符
%token <integer> INTEGER
%token <real> REAL
%token <string> IDENTIFIER STR
%token <comparator> OP

%token TAT QAQ PAP;
//定义语法
%%
pap: OP {if ($1==EQ_OP)printf("tat\n");else printf("qaq\n");};
qaq: REAL {printf("%f\n", $1);};
tat: INTEGER REAL {printf("%d %f\n", $1, $2);};
%%


void yyerror(const char *s, ...)
{
     fprintf(stderr, "%s\n", s);
}

int yywrap()
{
    return 1;
}
int main(){
    yyparse();
}
