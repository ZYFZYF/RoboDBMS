//
// Created by 赵鋆峰 on 2019/10/27.
//
#include <iostream>
#include "../def.h"

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
    return 0;
}