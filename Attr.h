//
// Created by 赵鋆峰 on 2019/12/3.
//

#ifndef ROBODBMS_TYPE_H
#define ROBODBMS_TYPE_H

#include <algorithm>
#include <iostream>
#include "SP/SP_Manager.h"
#include <cstring>

struct Date {
    int year;
    short month;
    short day;
};

struct Varchar {
    int offset;
    int length;
    char spName[MAX_NAME_LENGTH];

    void getData(char *pData) {
        SP_Handle spHandle;
        SP_Manager::OpenStringPool(spName, spHandle);
        spHandle.GetStringData(pData, offset, length);
        pData[length] = 0;
        SP_Manager::CloseStringPool(spHandle);
    }
};

enum AttrType {
    INT,
    FLOAT,
    STRING,
    DATE,
    VARCHAR,
    ATTRARRAY
};

union AttrValue {
    int intValue;
    float floatValue;
    char stringValue[MAX_CHAR_LENGTH + 1];
    Date dateValue;
    Varchar varcharValue;
    char *charValue;
};


//
// Comparison operators
//
enum CompOp {
    NO_OP,                                      // no comparison
    EQ_OP, NE_OP, LT_OP, GT_OP, LE_OP, GE_OP    // binary atomic operators
};

#endif //ROBODBMS_TYPE_H
