%{
#include<cstdio>
#include "../Attr.h"
#include "../PS/PS_Node.h"
#include "../PS/PS_ShowDatabases.h"
extern int yylex (void);
void yyerror(const char *s, ...);
%}
//让yylval不仅返回int数据
%union{
  int integer;
  float real;
  char * str;
  enum CompOp comparator;
  PS_Node *node;
}

//定义标识符
%token <integer> INTEGER
%token <real> REAL
%token <string> IDENTIFIER STR
%token <comparator> OP

%token SHOW USE CREATE DROP UPDATE INSERT ALTER SELECT ADD
%token DATABASES DATABASE TABLES TABLE INDEX
%token ON SET WHERE INTO

//定义语法
%%
sql:SHOW DATABASES ';'{
	auto x = new PS_ShowDatabases();
	x->run();
	YYACCEPT;
};
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
