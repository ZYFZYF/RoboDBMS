//
// Created by 赵鋆峰 on 2019/11/16.
//

#ifndef ROBODBMS_UTILS_H
#define ROBODBMS_UTILS_H


#include "../def.h"

class Utils {
public:
    static bool Compare(void *value1, void *value2, AttrType attrType, int attrLength, CompOp compOp);

    static void GetMinimumValue(void *value, AttrType attrType);

    static void GetMaximumValue(void *value, AttrType attrType);
};


#endif //ROBODBMS_UTILS_H
