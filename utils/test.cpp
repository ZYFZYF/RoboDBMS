//
// Created by 赵鋆峰 on 2020/1/10.
//
#include "Utils.h"
#include <iostream>

using namespace std;

void test(std::string a, std::string b) {
    cout << a << " like " << b << " is " << Utils::like(a, b) << endl;
}

int main() {
    test("abcde", "abcde%");
    test("a", "_");
    test("aaaaa", "_");
    test("aaaaa", "%");
    test("abcde", "%");
    test("abcde", "ab[cEFFfaa]de");
    test("abcde", "ab[cEFFfaade");
    test("abcde", "%c%");
    test("abcde", "%e");
    test("abcde", "a%");
    test("abcde", "[abc]%");
    test("abcde", "abcd?de");
    test("abcde", "ab[^b]de");
    test("abcde", "ab[^c]de");
    test("abcde", "abc_de");
    test("abcde", "abc_?de");
    test("abcde", "abc[abc]?de");
    test("ab_de", "ab[_]de");
    test("abcde", "ab[_]de");
    test("ab]de", "ab]de");
    test("ab_de", "ab]de");
    test("ab[de", "ab[de");
    test("ab[de", "ab[[]de");
    return 0;
}
