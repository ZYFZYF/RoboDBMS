%{
#include<cstdio>
#include "../Attr.h"
#include "../PS/PS_Node.h"
#include "../PS/PS_ShowDatabases.h"
#include "../SM/SM_Manager.h"
#include "../utils/PrintError.h"
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
%token <str> IDENTIFIER STR
%token <comparator> OP

%token SHOW USE CREATE DROP UPDATE INSERT ALTER SELECT ADD QUIT
%token DATABASES DATABASE TABLES TABLE INDEX
%token ON SET WHERE INTO

//定义语法
%%

CommondList	: 	CommondList Commond
		| 	Commond ;

Commond	: 	DDL | DML | HELP;

DDL 	: 	CreateDatabase
	| 	DropDatabase;

DML	: 	WHERE;

CreateDatabase	:	CREATE DATABASE IDENTIFIER ';'{
				DO(SM_Manager::Instance().CreateDb($3));
			};

DropDatabase	:	DROP DATABASE IDENTIFIER ';'{
         			DO(SM_Manager::Instance().DropDb($3));
         		};

HELP 	: 	SHOW DATABASES ';'{
			DO(SM_Manager::Instance().ShowDatabases());
		}
	|	QUIT ';'{
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
