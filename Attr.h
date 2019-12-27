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

    bool isValid() {
        if (month < 0 || month > 12)return false;
        if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
            return day >= 1 && day <= 31;
        if (month == 4 || month == 6 || month == 9 || month == 11)return day >= 1 && day <= 30;
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))return day >= 1 && day <= 29;
        return day >= 1 && day <= 28;
    }

    bool operator==(Date &date) {
        return year == date.year && month == date.month && day == month.day;
    }
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
    UNKNOWN,
    BOOL,
    INT,
    FLOAT,
    STRING,
    DATE,
    VARCHAR,
    ATTRARRAY
};

struct AttrValue {
    bool isNull;
    bool boolValue;
    int intValue;
    float floatValue;
    char stringValue[MAX_CHAR_LENGTH + 1];
    Date dateValue;
    Varchar varcharValue;
    char *charValue;
};

enum Operator {
    NO_OP, EQ_OP, NE_OP, LT_OP, GT_OP, LE_OP, GE_OP,

    PLUS_OP, MINUS_OP, MUL_OP, DIV_OP, MOD_OP,
    NOT_OP, AND_OP, OR_OP
};

#endif //ROBODBMS_TYPE_H
