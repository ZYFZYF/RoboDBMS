%{
#include<cstdio>
extern int yylex (void);
extern int yywrap(void);
void yyerror(const char *s, ...);
%}

%token TAT QAQ PAP;
%%
tat: TAT {printf("TATTATTAT\n");};
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
