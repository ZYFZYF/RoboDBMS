%{
#include<cstdio>
#include<vector>
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
  ColumnDesc columnDesc;
  AttrValue attrValue;
  std::vector<ColumnDesc> *columnList;
}

//定义标识符
%token <integer> INTEGER
%token <real> REAL
%token <str> IDENTIFIER STR
%token <comparator> OP

%token SHOW DESC USE CREATE DROP UPDATE INSERT ALTER SELECT ADD QUIT
%token DATABASES DATABASE TABLES TABLE INDEX PRIMARY KEY DEFAULT REFERENCES
%token P_ON P_SET P_WHERE P_INTO P_NOT P_NULL
%token T_INT T_BIGINT T_CHAR T_VARCHAR T_DATE T_DECIMAL T_NUMERIC

//定义语法中需要的节点的类型
%type <columnDesc> Column ColumnType NotNull DefaultValue PrimaryKey ForeignKey
%type <columnList> ColumnList
%type <attrValue> Value

//定义语法
%%

CommondList	: 	CommondList Commond
		| 	Commond ;

Commond	: 	DDL | DML | HELP;

DDL 	: 	CreateDatabase
	| 	DropDatabase
	| 	CreateTable;

DML	: 	P_WHERE;

HELP 	: 	SHOW DATABASES ';'{
			DO(SM_Manager::Instance().ShowDatabases());
		}
	|	USE DATABASE IDENTIFIER ';' {
			DO(SM_Manager::Instance().UseDb($3));
		};
	|	SHOW TABLES ';' {
			DO(SM_Manager::Instance().ShowTables());
		};
	|	DESC TABLE IDENTIFIER';' {
			DO(SM_Manager::Instance().DescTable($3));
		};
	|	QUIT ';'{
			YYACCEPT;
		};

CreateDatabase	:	CREATE DATABASE IDENTIFIER ';'{
				DO(SM_Manager::Instance().CreateDb($3));
			};

DropDatabase	:	DROP DATABASE IDENTIFIER ';'{
         			DO(SM_Manager::Instance().DropDb($3));
         		};

CreateTable	:	CREATE TABLE IDENTIFIER '(' ColumnList ')' ';'{
				//printf("%d\n",$5->size());
				DO(SM_Manager::Instance().CreateTable($3, $5));
			};

ColumnList	:	ColumnList ',' Column{
				$$ = $1;
				$$->push_back($3);
			}
		|	Column{
				$$ = new std::vector<ColumnDesc>;
				$$->push_back($1);
			};

Column		:	IDENTIFIER ColumnType NotNull DefaultValue PrimaryKey ForeignKey{
				strcpy($$.name, $1);
				$$.attrType = $2.attrType;
				$$.attrLength = $2.attrLength;
				$$.stringMaxLength = $2.stringMaxLength;
				$$.integerLength = $2.integerLength;
				$$.decimalLength = $2.decimalLength;
				$$.allowNull = $3.allowNull;
				$$.hasDefaultValue = $4.hasDefaultValue;
				$$.defaultValue = $4.defaultValue;
				$$.isPrimaryKey = $5.isPrimaryKey;
				$$.hasForeignKey = $6.hasForeignKey;
				strcpy($$.foreignKeyTable, $6.foreignKeyTable);
				strcpy($$.foreignKeyColumn, $6.foreignKeyColumn);
				//printf("name = %s, attrLength = %d, stringMaxLength = %d, integerLength = %d, decimalLength = %d, allowNull = %d, hasDefaultValue = %d, defaultValue = %s, isPrimaryKey = %d, hasForeignKey = %d, foreignKeyTable = %s, foreignKeyColumn = %s\n",$$.name, $$.attrLength, $$.stringMaxLength, $$.integerLength, $$.decimalLength, $$.allowNull, $$.hasDefaultValue, &$$.defaultValue, $$.isPrimaryKey, $$.hasForeignKey, $$.foreignKeyTable, $$.foreignKeyColumn);
			};

ColumnType	:	T_INT
			{
				$$.attrType = INT;
				$$.attrLength = 4;
			}
		|	T_BIGINT
			{
				$$.attrType = INT;
				$$.attrLength = 4;
			}
		|	T_CHAR '(' INTEGER ')'
			{
				$$.attrType = STRING;
				$$.attrLength = $3;
			}
		| 	T_VARCHAR '(' INTEGER ')'
			{
				$$.attrType = VARCHAR;
				$$.attrLength = sizeof(Varchar);
				$$.stringMaxLength = $3;
			}
		|	T_DATE
			{
				$$.attrType = DATE;
				$$.attrLength = sizeof(Date);
			}
		|	T_DECIMAL
			{
				$$.attrType = FLOAT;
				$$.attrLength = 4;
			}
		|	T_NUMERIC '(' INTEGER ',' INTEGER ')'
			{
				$$.attrType = FLOAT;
				$$.attrLength = 4;
				$$.integerLength = $3;
				$$.decimalLength = $5;
			}
		;

NotNull		:	/* empty */
			{
				$$.allowNull = true;
			}
		|	P_NOT P_NULL
			{
				$$.allowNull = false;
			};
DefaultValue 	:	/* empty */
		|	{
				$$.hasDefaultValue = false;
				memset(&$$.defaultValue, 0, sizeof($$.defaultValue));
			}
		|	DEFAULT Value
			{
				$$.hasDefaultValue = true;
				$$.defaultValue = $2;
			};

Value		:	INTEGER
			{
				$$.intValue = $1;
			}
		|	REAL
			{
				$$.floatValue = $1;
			}
		|	STR
			{
				memcpy($$.stringValue, $1 + 1, strlen($1) - 2);
				$$.stringValue[strlen($1)-2] = '\0';
			};

PrimaryKey	:	/* empty */
			{
				$$.isPrimaryKey = false;
			}
		|	PRIMARY KEY
			{
				$$.isPrimaryKey = true;
			};

ForeignKey	:	/* empty */
			{
				$$.hasForeignKey = false;
				memset($$.foreignKeyTable, 0, sizeof($$.foreignKeyTable));
				memset($$.foreignKeyColumn, 0, sizeof($$.foreignKeyColumn));
			}
		|	REFERENCES IDENTIFIER '.' IDENTIFIER
			{
				$$.hasForeignKey = true;
				strcpy($$.foreignKeyTable, $2);
				strcpy($$.foreignKeyColumn, $4);
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
