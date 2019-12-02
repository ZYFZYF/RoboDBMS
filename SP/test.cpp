//
// Created by 赵鋆峰 on 2019/11/12.
//
#include <iostream>
#include <cassert>
#include <vector>
#include "SP_Manager.h"
#include "../utils/PrintError.h"
#include <cstring>
#include "SP_Constant.h"

using namespace std;

#define TEST_FILE_NAME "test.txt"
#define TEST_STRING "abcdefgh"
#define TEST_STRING_LENGTH 10
SP_Handle spHandle;

//测试StringPool的创建、打开
RC test1() {
    TRY(SP_Manager::CreateStringPool(TEST_FILE_NAME));
    TRY(SP_Manager::OpenStringPool(TEST_FILE_NAME, spHandle));


}

struct varchar {
    string str;
    int offset;
    int length;
};
std::vector<varchar> varchars;

string generateString() {
    string ret;
    int n = rand() % 23 + 1;
    for (int i = 0; i < n; i++) {
        char ch;
        if (rand() % 2 == 0) {
            ch = rand() % 26 + (rand() % 2 == 0 ? 'a' : 'A');
        } else {
            ch = rand() % 10 + '0';
        }
        ret.insert(0, 1, ch);
    }
    return ret;
}

//简单的测试
RC test2() {
    srand(time(0));
    char a[TEST_STRING_LENGTH] = "abcdefg", b[TEST_STRING_LENGTH];
    int offset;
    TRY(spHandle.InsertString(a, TEST_STRING_LENGTH, offset));
    assert(offset == SP_SPACE_HEADER_SIZE);
    TRY(spHandle.InsertString(a, TEST_STRING_LENGTH, offset));
    assert(offset == SP_SPACE_HEADER_SIZE * 2);
    TRY(spHandle.InsertString(a, TEST_STRING_LENGTH, offset));
    assert(offset == SP_SPACE_HEADER_SIZE * 3);
    TRY(spHandle.DeleteString(offset, TEST_STRING_LENGTH));
    TRY(spHandle.InsertString(a, TEST_STRING_LENGTH, offset));
    assert(offset == SP_SPACE_HEADER_SIZE * 3);
    TRY(spHandle.DeleteString(SP_SPACE_HEADER_SIZE * 2, TEST_STRING_LENGTH));
    TRY(spHandle.InsertString(a, TEST_STRING_LENGTH, offset));
    assert(offset == SP_SPACE_HEADER_SIZE * 2);
    TRY(spHandle.DeleteString(SP_SPACE_HEADER_SIZE, TEST_STRING_LENGTH));
    TRY(spHandle.DeleteString(SP_SPACE_HEADER_SIZE * 2, TEST_STRING_LENGTH));
    TRY(spHandle.DeleteString(SP_SPACE_HEADER_SIZE * 3, TEST_STRING_LENGTH));
    SP_SpaceHeader spSpaceHeader;
    spHandle.ReadSpaceHeader(spSpaceHeader, 0);
    assert(spSpaceHeader.nextOffset == SP_SPACE_HEADER_SIZE);
    assert(spSpaceHeader.prevOffset == NO_MORE_SPACE);
    return OK_RC;
}

//随机大数据测试
RC test3() {
    int num_iterations = 100;
    for (int i = 0; i < num_iterations; i++) {
        int op;
        if (i < num_iterations / 2) {
            op = 0;
        } else {
            op = rand() % 2 + 1;
        }
        if (op == 0) {
            varchar t;
            t.str = generateString();
            t.length = t.str.length();
            spHandle.InsertString(t.str.c_str(), t.length, t.offset);
            varchars.push_back(t);
            cout << "Insert string " + t.str << endl;
        } else if (op == 1) {
            int x = rand() % (varchars.size());
            cout << "Delete string " + varchars[x].str << endl;
            varchars.erase(varchars.begin() + x);
        } else if (op == 2) {
            int x = rand() % (varchars.size());
            varchar t;
            t.str = generateString();
            t.length = t.str.length();
            cout << "Update string " + varchars[x].str << " to " << t.str << endl;
            spHandle.UpdateString(t.str.c_str(), t.length, varchars[x].offset, varchars[x].length);
            varchars[x].str = t.str;
            varchars[x].length = t.length;
        }
    }
    cout << "we have totally " << varchars.size() << " strings" << endl;
    for (auto var:varchars) {
        char data[var.length + 1];
        data[var.length] = '\0';
        TRY(spHandle.GetStringData(data, var.offset, var.length));
        cout << var.str << ' ' << data << ' ' << var.offset << ' ' << var.length << endl;
        assert(strcmp(data, var.str.c_str()) == 0);
    }
    return OK_RC;
}

//测试StringPool的关闭、销毁
RC test4() {
    TRY(SP_Manager::CloseStringPool(spHandle));
    TRY(SP_Manager::DestroyStringPool(TEST_FILE_NAME));
}

int main() {
    RC rc;
    if (access(TEST_FILE_NAME, F_OK) != -1) {
        //这个文件本身存在的话要删除，以防后面创建出错
        unlink(TEST_FILE_NAME);
    }

    cout << "test1 starting ****************" << endl;
    if ((rc = test1())) {
        printError(rc);
        return 1;
    }
    cout << "test1 done ********************" << endl;
    cout << "test2 starting ****************" << endl;

    if ((rc = test2())) {
        printError(rc);
        return 1;
    }
    cout << "test2 done ********************" << endl;
    cout << "test3 starting ****************" << endl;

    if ((rc = test3())) {
        printError(rc);
        return 1;
    }
    cout << "test3 done ********************" << endl;

    cout << "test4 starting ****************" << endl;

    if ((rc = test4())) {
        printError(rc);
        return 1;
    }
    cout << "test4 done ********************" << endl;

    cout
            << "******************************** Congratulations to you! You have passed all tests! ********************************"
            << endl;
    return 0;
}
