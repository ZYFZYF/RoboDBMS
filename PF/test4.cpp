//
// Created by 赵鋆峰 on 2019/10/27.
//
#include <iostream>
#include <cstring>
#include "../Constant.h"
#include "../Attr.h"

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
    if ((buffer = getcwd(NULL, 0)) == NULL) {
        perror("getcwd error");
    } else {
        printf("%s\n", buffer);
        free(buffer);
    }

    char temp[100];
    char format[100];
    sprintf(format, "%%%d.%df", 10, 2);
    sprintf(temp, format, 1000.222222);
    printf("%s %s", format, temp);
    sprintf(temp, format, 10000000000.222222);
    printf("%s %s", format, temp);
    std::string s;
    s.append(10, 'a');
    cout << s << endl;
    s.append(0, 'a');
    cout << s << endl;
    s.append('a', 10);
    cout << s << endl;

    char p[] = "11102";
    cout << atoi(p) << endl;
    char q[] = "test";
    cout << atoi(q) << endl;

    char *endPtr;
    strtol(p, &endPtr, 10);
    cout << endPtr << ' ' << (endPtr == nullptr) << ' ' << strlen(endPtr) << endl;
    strtol(q, &endPtr, 10);
    cout << endPtr << ' ' << (endPtr == nullptr) << ' ' << strlen(endPtr) << endl;
    char v[] = "11.11";
    strtol(v, &endPtr, 10);
    cout << endPtr << ' ' << (endPtr == nullptr) << ' ' << strlen(endPtr) << endl;

    char x[100];
    sprintf(x, "%d", 100);
    string y = x;
    cout << x << ' ' << y.length() << endl;

    char test[] = "2010-02-221";
    Date date{};
    date.year = strtol(test, &endPtr, 10);
    if (endPtr != test + 4 || endPtr[0] != '-') cout << "error" << endl;
    date.month = strtol(test + 5, &endPtr, 10);
    if (endPtr != test + 7 || endPtr[0] != '-')cout << "error" << endl;
    date.day = strtol(test + 8, &endPtr, 10);
    if (endPtr != test + 10 || strlen(endPtr) != 0)cout << "error" << endl;
    cout << date.year << ' ' << date.month << ' ' << date.day << endl;
    return 0;
}