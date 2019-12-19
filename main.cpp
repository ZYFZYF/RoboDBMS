#include <stdio.h>

extern FILE *yyin;

extern int yyparse();

int main(int argc) {
    const char test_sql[] = "../test/db.sql";
    yyin = fopen(test_sql, "rb");
    if (NULL == yyin) {
        printf("Open file failed: %s\n", test_sql);
        return 0;
    } else {
        printf("Open file succeed\n");
    }

    while (!feof(yyin)) {
        yyparse();
    }

    fclose(yyin);
    yyin = NULL;

    return 0;
}