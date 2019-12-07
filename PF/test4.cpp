//
// Created by 赵鋆峰 on 2019/10/27.
//
#include <iostream>
#include "../Constant.h"

using namespace std;
enum test {
    OK = 10, TAT, QAQ, PAT
};

int main() {
    cout << "Hello world" << endl;
    test a = OK;
    test b = TAT;
    cout << a << ' ' << b << endl;
    cout << (a == 10) << endl;
    cout << OK_RC << endl;
    if (OK_RC) {
        cout << "OK_RC is true" << endl;
    }
    if (PF_HASHNOTFOUND) {
        cout << "Other is ture" << endl;
    }
    char *buffer;
    //也可以将buffer作为输出参数
    if((buffer = getcwd(NULL, 0)) == NULL)
    {
        perror("getcwd error");
    }
    else
    {
        printf("%s\n", buffer);
        free(buffer);
    }

    return 0;
}