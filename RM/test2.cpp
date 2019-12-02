//
// Created by 赵鋆峰 on 2019/10/28.
//
#include <iostream>
#include <cassert>
#include "RM_Constant.h"
#include "RM_FileHandle.h"

using namespace std;

int main() {
    cout << RM_PAGE_HEADER_SIZE << endl;
    int y = 8;
    cout << y << ' ' << (~y) << endl;
    RM_FileHandle rfh;
    char *bitmap = new char[100];
    int size = 100;
    rfh.ResetBitmap(bitmap, size);
    int x;
    rfh.FindFirstZero(bitmap, size, x);
    assert(x == 0);
    rfh.SetBit(bitmap, size, 0);
    rfh.FindFirstZero(bitmap, size, x);
    assert(x == 1);
    rfh.ClearBit(bitmap, size, 0);
    rfh.FindFirstZero(bitmap, size, x);
    assert(x == 0);
    for (int i = 0; i < 50; i++) {
        rfh.SetBit(bitmap, size, i);
    }
    rfh.FindFirstZero(bitmap, size, x);
    assert(x == 50);
    if (int w = 1) {
        cout << w << endl;
    }
    if (int w = 2) {
        cout << w << endl;
    }
    if (int w = 3) {
        if (int w = 4) {
            cout << w << endl;
        }
        cout << w << endl;
    }

    cout << "sizeof(RM_RID) is " << sizeof(class RM_RID) << endl;
    RM_RID rid1(2, 2);
    RM_RID rid2(2, 2);
    RM_RID rid3(1, 2);
    RM_RID rid4(3, 2);
    cout << "rid equal == " << (rid1 == rid2) << endl;
    cout << "(2,2) < (2,2) is " << (rid1 < rid2) << endl;
    cout << "(1,2) < (3,2) is " << (rid3 < rid4) << endl;
    cout << "(3,2) >= (2,2) is " << (rid4 >= rid2) << endl;
    cout << "(2,2) > (2,2) is " << (rid1 > rid2) << endl;

    int *z = new int[10];
    void *temp = z;
    cout << z << ' ' << z + 1 << endl;
    return 0;
}
