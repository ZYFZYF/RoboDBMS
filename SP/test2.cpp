//
// Created by 赵鋆峰 on 2019/12/3.
//

#include "SP_Manager.h"
#include "../Attr.h"
#include "../utils/Utils.h"
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    SP_Handle spHandle;
    SP_Manager::CreateStringPool("test.sp");
    vector<int> ind;
    Varchar vars[100];
    int n = 20;
    for (int i = 0; i < n; i++) {
        ind.push_back(i);
    }
    random_shuffle(ind.begin(), ind.end());
    //在字符串池中加入这些字符串
    SP_Manager::OpenStringPool("test.sp", spHandle);
    for (int i = 0; i < n; i++) {
        char x[10];
        sprintf(x, "%d", ind[i]);
        if (i < 10)vars[i].length = 1;
        else vars[i].length = 2;
        strcpy(vars[i].spName, "test.sp");
        spHandle.InsertString(x, vars[i].length, vars[i].offset);
    }
    SP_Manager::CloseStringPool(spHandle);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            char x[100];
            vars[i].getData(x);
            cout << ind[i] << " get data is " << x << endl;
            cout << ind[i] << " == " << ind[j] << ' '
                 << Utils::Compare(&vars[i], &vars[j], VARCHAR, sizeof(Varchar), EQ_OP) << endl;
            cout << ind[i] << " != " << ind[j] << ' '
                 << Utils::Compare(&vars[i], &vars[j], VARCHAR, sizeof(Varchar), NE_OP) << endl;
            cout << ind[i] << " < " << ind[j] << ' '
                 << Utils::Compare(&vars[i], &vars[j], VARCHAR, sizeof(Varchar), LT_OP) << endl;
            cout << ind[i] << " <= " << ind[j] << ' '
                 << Utils::Compare(&vars[i], &vars[j], VARCHAR, sizeof(Varchar), LE_OP) << endl;
            cout << ind[i] << " > " << ind[j] << ' '
                 << Utils::Compare(&vars[i], &vars[j], VARCHAR, sizeof(Varchar), GT_OP) << endl;
            cout << ind[i] << " >= " << ind[j] << ' '
                 << Utils::Compare(&vars[i], &vars[j], VARCHAR, sizeof(Varchar), GE_OP) << endl;
        }


    SP_Manager::DestroyStringPool("test.sp");
    return 0;
}