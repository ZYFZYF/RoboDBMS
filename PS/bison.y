%{
#include<cstdio>
#include<vector>
#include "../Attr.h"
#include "../PS/PS_Node.h"
#include "../PS/PS_ShowDatabases.h"
#include "../PS/PS_Expr.h"
#include "../SM/SM_Manager.h"
#include "../utils/PrintError.h"
#include "../QL/QL_Manager.h"
extern int yylex (void);
void yyerror(const char *s, ...);

%}
//让yylval不仅返回int数据
%union{
  int integer;
  float real;
  char * str;
  enum Operator comparator;
  PS_Node *node;
  PS_Expr *expr;
  std::vector<PS_Expr> *conditionList;
  ColumnDesc columnDesc;
  AttrValue attrValue;
  std::vector<AttrValue> *attrValueList;
  std::vector<ColumnDesc> *columnList;
  std::vector<const char *> *identifierList;
  std::pair<std::string, PS_Expr> *assignExpr;
  std::vector<std::pair<std::string, PS_Expr> > *assignExprList;
}

//定义标识符
%token <str> IDENTIFIER STR STR_INTEGER STR_REAL
%token <comparator> OP

%token SHOW DESC USE CREATE DROP UPDATE INSERT DELETE ALTER SELECT ADD QUIT
%token COUNT
%token DATABASES DATABASE TABLES TABLE INDEX PRIMARY KEY DEFAULT REFERENCES FOREIGN CONSTRAINT
%token P_ON P_SET P_WHERE P_INTO P_NOT P_NULL P_VALUES P_FROM P_IS
%token T_INT T_BIGINT T_CHAR T_VARCHAR T_DATE T_DECIMAL
%token C_AND C_OR

//定义语法中需要的节点的类型
%type <integer> INTEGER
%type <real> REAL
%type <columnDesc> Column ColumnType NotNull DefaultValue PrimaryKey ForeignKey
%type <columnList> ColumnDescList
%type <attrValue> ConstValue
%type <attrValueList> ConstValueList
%type <identifierList> NameList
%type <expr> BoolExpr ValueExpr LowerValueExpr LeafValueExpr
%type <conditionList> WhereClause ConditionList
%type <assignExpr> AssignExpr
%type <assignExprList> SetClause

//定义语法
%%

CommondList	: 	CommondList Commond
		| 	Commond ;

Commond	: 	DDL | DML | HELP;

DDL 	: 	CreateDatabase
	| 	DropDatabase
	| 	CreateTable
	|	DropTable
	|	AddPrimaryKey
	|	AddForeignKey
	|	DropPrimaryKey
	|	DropForeignKey
	|	AddIndex
	|	DropIndex
	;

DML	: 	InsertRow
	|	InsertFromFile
	|	Delete
	|	Update
	|	Select
	;

HELP 	: 	SHOW DATABASES ';'{
			DO(SM_Manager::Instance().ShowDatabases());
		}
	|	USE DATABASE IDENTIFIER ';' {
			DO(SM_Manager::Instance().UseDb($3));
		}
	|	SHOW TABLES ';' {
			DO(SM_Manager::Instance().ShowTables());
		}
	|	DESC TABLE IDENTIFIER';' {
			DO(SM_Manager::Instance().DescTable($3));
		}
	|	SHOW TABLE IDENTIFIER';' {
			DO(SM_Manager::Instance().ShowTable($3));
		}
	| 	COUNT TABLE IDENTIFIER ';' {
			DO(QL_Manager::Instance().Count($3));
		}
	|	QUIT ';'{
			YYACCEPT;
		};

CreateDatabase	:	CREATE DATABASE IDENTIFIER ';'
			{
				DO(SM_Manager::Instance().CreateDb($3));
			};

DropDatabase	:	DROP DATABASE IDENTIFIER ';'
			{
         			DO(SM_Manager::Instance().DropDb($3));
         		};

CreateTable	:	CREATE TABLE IDENTIFIER '(' ColumnDescList ')' ';'
			{
				//printf("%d\n",$5->size());
				DO(SM_Manager::Instance().CreateTable($3, $5));
			};

DropTable	:	DROP TABLE IDENTIFIER ';'
			{
				DO(SM_Manager::Instance().DropTable($3));
			}

ColumnDescList	:	ColumnDescList ',' Column
			{
				$$ = $1;
				$$->push_back($3);
			}
		|	Column
			{
				$$ = new std::vector<ColumnDesc>;
				$$->push_back($1);
			};

Column		:	IDENTIFIER ColumnType NotNull DefaultValue PrimaryKey ForeignKey
			{
				strcpy($$.name, $1);
				$$.attrType = $2.attrType;
				$$.attrLength = $2.attrLength;
				$$.stringMaxLength = $2.stringMaxLength;
				$$.integerLength = $2.integerLength;
				$$.decimalLength = $2.decimalLength;
				$$.allowNull = $3.allowNull;
				$$.hasDefaultValue = $4.hasDefaultValue;
				$$.defaultValue = $4.defaultValue;
				//printf("qaq%d%s",$$.hasDefaultValue, $$.defaultValue.charValue);
				$$.isPrimaryKey = $5.isPrimaryKey;
				$$.hasForeignKey = $6.hasForeignKey;
				strcpy($$.primaryKeyTable, $6.primaryKeyTable);
				strcpy($$.primaryKeyColumn, $6.primaryKeyColumn);
				//printf("name = %s, attrLength = %d, stringMaxLength = %d, integerLength = %d, decimalLength = %d, allowNull = %d, hasDefaultValue = %d, defaultValue = %s, isPrimaryKey = %d, hasForeignKey = %d, primaryKeyTable = %s, primaryKeyColumn = %s\n",$$.name, $$.attrLength, $$.stringMaxLength, $$.integerLength, $$.decimalLength, $$.allowNull, $$.hasDefaultValue, &$$.defaultValue, $$.isPrimaryKey, $$.hasForeignKey, $$.primaryKeyTable, $$.primaryKeyColumn);
			};

ColumnType	:	T_INT
			{
				$$.attrType = INT;
				$$.attrLength = 4;
			}
		|	T_CHAR '(' INTEGER ')'
			{
				$$.attrType = STRING;
				$$.attrLength = $3 + 1;
			}
		| 	T_VARCHAR '(' INTEGER ')'
			{
				$$.attrType = VARCHAR;
				$$.attrLength = sizeof(Varchar);
				$$.stringMaxLength = $3 + 1;
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
		|	T_DECIMAL '(' INTEGER ',' INTEGER ')'
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
			{
				$$.hasDefaultValue = false;
				memset(&$$.defaultValue, 0, sizeof($$.defaultValue));
			}
		|	DEFAULT ConstValue
			{
				$$.hasDefaultValue = true;
				$$.defaultValue = $2;
			};

ConstValueList	:	ConstValueList ',' ConstValue
			{
				$$ = $1;
				$$->push_back($3);
			}
		|	ConstValue
			{
				$$ = new std::vector<AttrValue>;
				$$->push_back($1);
			}

ConstValue	:	/* empty */
			{
				$$.isNull = true;
			}
		|	P_NULL
			{
				$$.isNull = true;
			}
		|	STR_INTEGER
			{
				$$.isNull = false;
				$$.charValue = $1;
			}
		|	STR_REAL
			{
				$$.isNull = false;
				$$.charValue = $1;
			}
		|	STR
			{
				$$.isNull = false;
				$$.charValue = $1;
			}
		;

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
				memset($$.primaryKeyTable, 0, sizeof($$.primaryKeyTable));
				memset($$.primaryKeyColumn, 0, sizeof($$.primaryKeyColumn));
			}
		|	REFERENCES IDENTIFIER '.' IDENTIFIER
			{
				$$.hasForeignKey = true;
				strcpy($$.primaryKeyTable, $2);
				strcpy($$.primaryKeyColumn, $4);
			};

AddPrimaryKey	:	ALTER TABLE IDENTIFIER ADD PRIMARY KEY '(' NameList ')' ';'
			{
				DO(SM_Manager::Instance().AddPrimaryKey($3, $8));
			};

AddForeignKey	:	ALTER TABLE IDENTIFIER ADD CONSTRAINT IDENTIFIER FOREIGN KEY '(' NameList ')' REFERENCES IDENTIFIER '(' NameList ')' ';'
			{
				DO(SM_Manager::Instance().AddForeignKey($6, $3, $10, $13, $15));
			};

NameList	:	NameList ',' IDENTIFIER
			{
				$$ = $1;
				$$->push_back($3);
			}
		|	IDENTIFIER
			{
				$$ = new std::vector<const char *>;
				$$->push_back($1);
			};

DropPrimaryKey	:	ALTER TABLE IDENTIFIER DROP PRIMARY KEY ';'
			{
				DO(SM_Manager::Instance().DropPrimaryKey($3));
			};

DropForeignKey	:	ALTER TABLE IDENTIFIER DROP FOREIGN KEY IDENTIFIER ';'
			{
				DO(SM_Manager::Instance().DropForeignKey($3,$7));
			};

AddIndex	:	ALTER TABLE IDENTIFIER ADD INDEX IDENTIFIER '(' NameList ')' ';'
			{
				DO(SM_Manager::Instance().AddIndex($3,$6,$8));
			};

DropIndex	:	ALTER TABLE IDENTIFIER DROP INDEX IDENTIFIER ';'
			{
				DO(SM_Manager::Instance().DropIndex($3,$6));
			};

InsertRow	:	INSERT P_INTO IDENTIFIER P_VALUES '(' ConstValueList ')' ';'
			{
				DO(QL_Manager::Instance().Insert($3, nullptr, $6));
			}
		|	INSERT P_INTO IDENTIFIER '(' NameList ')' P_VALUES '(' ConstValueList ')' ';'
			{
				DO(QL_Manager::Instance().Insert($3, $5, $9));
			}
		;

InsertFromFile 	:	INSERT P_INTO IDENTIFIER P_FROM STR ';'
			{
				DO(QL_Manager::Instance().Insert($3, $5));
			}
		;

Delete		:	DELETE P_FROM IDENTIFIER WhereClause ';'
			{
				DO(QL_Manager::Instance().Delete($3, $4));
			}
		;

Update		:	UPDATE IDENTIFIER P_SET SetClause WhereClause ';'
			{
				DO(QL_Manager::Instance().Update($2, $4, $5));
			}
		;

Select 		:	Select NamedColumnList

SetClause	:	SetClause ',' AssignExpr
			{
				$$ = $1;
				$$->emplace_back(*$3);
			}
		|	AssignExpr
			{
				$$ = new std::vector<std::pair<std::string, PS_Expr> >;
				$$->emplace_back(*$1);
			}
		;

AssignExpr	:	IDENTIFIER '=' ValueExpr
			{
				$$ = new std::pair<std::string, PS_Expr>(std::string($1), *$3);
			}
		;

WhereClause	:	/* empty */
			{
				$$ = new std::vector<PS_Expr>;
				$$->emplace_back(PS_Expr(true));
			}
		|	P_WHERE ConditionList
			{
				$$ = $2;
			}
		;

ConditionList	:	ConditionList C_AND BoolExpr
			{
				$$ = $1;
				$$->emplace_back(*$3);
			}
		|	BoolExpr
			{
				$$ = new std::vector<PS_Expr>;
				$$->emplace_back(*$1);
			}
		;

BoolExpr	:	BoolExpr C_OR BoolExpr
			{
				$$ = new PS_Expr($1, OR_OP, $3);
			}
		|	ValueExpr '=' '=' ValueExpr
			{
				$$ = new PS_Expr($1, EQ_OP, $4);
			}
		|	ValueExpr '=' ValueExpr
			{
				$$ = new PS_Expr($1, EQ_OP, $3);
			}
		|	ValueExpr '!' '=' ValueExpr
			{
				$$ = new PS_Expr($1, NE_OP, $4);
			}
		|	ValueExpr '<' '>' ValueExpr
			{
				$$ = new PS_Expr($1, NE_OP, $4);
			}
		|	ValueExpr '<' ValueExpr
			{
				$$ = new PS_Expr($1, LT_OP, $3);
			}
		|	ValueExpr '<' '=' ValueExpr
			{
				$$ = new PS_Expr($1, LE_OP, $4);
			}
		|	ValueExpr '>' ValueExpr
			{
				$$ = new PS_Expr($1, GT_OP, $3);
			}
		|	ValueExpr '>' '=' ValueExpr
			{
				$$ = new PS_Expr($1, GE_OP, $4);
			}
		|	P_NOT '(' BoolExpr ')'
                 	{
                        	$$ = new PS_Expr(nullptr, NOT_OP, $3);
                	}
                |	ValueExpr P_IS P_NULL
                        {
                        	$$ = new PS_Expr($1, EQ_OP, new PS_Expr());
                        }
                |	ValueExpr P_IS P_NOT P_NULL
                        {
                        	$$ = new PS_Expr($1, NE_OP, new PS_Expr());
                        }
		;

ValueExpr	:	ValueExpr '+' LowerValueExpr
			{
				$$ = new PS_Expr($1, PLUS_OP, $3);
			}
		|	ValueExpr '-' LowerValueExpr
			{
				$$ = new PS_Expr($1, MINUS_OP, $3);
			}
		|	LowerValueExpr
			{
				$$ = $1;
			}
		;

LowerValueExpr	:	LowerValueExpr '*' LeafValueExpr
			{
				$$ = new PS_Expr($1, MUL_OP, $3);
			}
		|	LowerValueExpr '/' LeafValueExpr
			{
				$$ = new PS_Expr($1, DIV_OP, $3);
			}
		|	LowerValueExpr '%' LeafValueExpr
			{
				$$ = new PS_Expr($1, MOD_OP, $3);
			}
		|	LeafValueExpr
			{
				$$ = $1;
			}
		;

LeafValueExpr	:	P_NULL
			{
				$$ = new PS_Expr();
			}
		|	INTEGER
			{
				$$ = new PS_Expr($1);
			}
		|	REAL
			{
				$$ = new PS_Expr($1);
			}
		|	STR
			{
				$$ = new PS_Expr($1);
			}
		|	IDENTIFIER
			{
				$$ = new PS_Expr(nullptr, $1);
			}
		|	IDENTIFIER '.' IDENTIFIER
			{
				$$ = new PS_Expr($1, $3);
			}
		|	'(' ValueExpr ')'
                	{
                		$$ = $2;
                	}
		;

INTEGER 	:	STR_INTEGER
			{
				$$ = atoi($1);
			}
		;

REAL		:	STR_REAL
			{
				$$ = atof($1);
			}
		;
%%


void yyerror(const char *s, ...)
{
     fprintf(stderr, "%s\n", s);
}

int yywrap()
{
    return 1;
}
