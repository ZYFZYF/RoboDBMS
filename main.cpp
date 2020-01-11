#include <cstdio>
#include <cstring>

extern FILE *yyin;

extern int yyparse();

int main(int argc, char *argv[]) {
    //有任意参数从文件读，否则从标准输入读
    if (argc > 1) {
        const char test_sql[] = "../test/test_multi_join.sql";
        yyin = fopen(test_sql, "r");
        if (yyin == nullptr) {
            printf("Open file  %s\n failed", test_sql);
            return 0;
        }
        while (!feof(yyin)) {
            yyparse();
        }
        fclose(yyin);
    } else {
        yyparse();
    }
    return 0;
}