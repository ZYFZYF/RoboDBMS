//
// Created by 赵鋆峰 on 2019/11/16.
//

#ifndef ROBODBMS_UTILS_H
#define ROBODBMS_UTILS_H


#include <string>
#include "../Constant.h"
#include "../Attr.h"


class Utils {
public:

    static std::string getIndexFileName(const char *fileName, int indexNo);

    static std::string getStringPoolFileName(const char *fileName);

    static std::string getRecordFileName(const char *fileName);

    static bool Compare(void *value1, void *value2, AttrType attrType, int attrLength, Operator compOp);

    //类似strcmp，<返回负数，==返回0，＞返回正数
    static int Cmp(void *value1, void *value2, AttrType attrType, int attrLength);

    static void GetMinimumValue(void *value, AttrType attrType);

    static void GetMaximumValue(void *value, AttrType attrType);

    static std::string getAbsolutePath(const char *filename);

    static bool isComparable(Operator op);

    static bool isArithmetic(Operator op);

    static bool isLogic(Operator op);

};


#endif //ROBODBMS_UTILS_H
