//
// Created by 赵鋆峰 on 2019/10/28.
//
#include <iostream>
#include <cassert>
#include "def.h"
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
    return 0;
}
