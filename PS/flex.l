%{
#include<cstdio>
#include<iostream>
#include "bison.h"
%}

%%
start printf("QAQ");
stop printf("TAT");
0 return TAT;
[1-9][0-9]* printf("PAP");
[a-zA-Z][a-zA-Z0-9]* printf("WORD");
%%