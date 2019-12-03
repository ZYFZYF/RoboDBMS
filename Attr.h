//
// Created by 赵鋆峰 on 2019/12/3.
//

#ifndef ROBODBMS_TYPE_H
#define ROBODBMS_TYPE_H

#include <algorithm>
#include <iostream>
#include "SP/SP_Manager.h"
#include "cstring"

struct Date {
    int year;
    short month;
    short day;

    bool operator==(Date &date) {
        return year == date.year && month == date.month && day == date.day;
    }

    bool operator<(Date &date) {
        if (year == date.year) {
            if (month == date.month) {
                return day < date.day;
            } else {
                return month < date.month;
            }
        } else {
            return year < date.year;
        }
    }

    bool operator<=(Date &date) {
        if (year == date.year) {
            if (month == date.month) {
                return day <= date.day;
            } else {
                return month < date.month;
            }
        } else {
            return year < date.year;
        }
    }

    bool operator!=(Date &date) {
        return !((*this) == date);
    }

    bool operator>(Date &date) {
        return !((*this) <= date);
    }

    bool operator>=(Date &date) {
        return !((*this) < date);
    }
};

struct Varchar {
    int offset;
    int length;
    char spName[MAX_NAME_LENGTH];

    bool operator==(Varchar &varchar) {
        char valueLeft[length];
        getData(valueLeft);

        char valueRight[varchar.length];
        varchar.getData(valueRight);

        bool ret;
        if (length != varchar.length) {
            ret = false;
        } else {
            ret = (memcmp(valueLeft, valueRight, length) == 0);
        }
//        if (length != varchar.length) {
//            std::cout << valueLeft << " == " << valueRight << ' ' << ret << std::endl;
//        }
        return ret;
    }

    bool operator<(Varchar &varchar) {
        char valueLeft[length];
        getData(valueLeft);

        char valueRight[varchar.length];
        varchar.getData(valueRight);

        bool ret;
        int cmp = memcmp(valueLeft, valueRight, std::min(length, varchar.length));
        if (cmp < 0) {
            ret = true;
        } else if (cmp == 0) {
            ret = length < varchar.length;
        } else {
            ret = false;
        }
//        if (length != varchar.length) {
//            std::cout << valueLeft << "  < " << valueRight << ' ' << ret << std::endl;
//        }
        return ret;
    }

    bool operator<=(Varchar &varchar) {
        char valueLeft[length];
        getData(valueLeft);

        char valueRight[varchar.length];
        varchar.getData(valueRight);

        bool ret;
        int cmp = memcmp(valueLeft, valueRight, std::min(length, varchar.length));
        if (cmp < 0) {
            ret = true;
        } else if (cmp == 0) {
            ret = length <= varchar.length;
        } else {
            ret = false;
        }
//        if (length != varchar.length) {
//            std::cout << valueLeft << " <= " << valueRight << ' ' << ret << std::endl;
//        }
        return ret;
    }

    bool operator!=(Varchar &varchar) {
        return !((*this) == varchar);
    }

    bool operator>=(Varchar &varchar) {
        return !((*this) < varchar);
    }

    bool operator>(Varchar &varchar) {
        return !((*this) <= varchar);
    }

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
    int intVersion;
    float floatVersion;
    char stringValue[MAX_CHAR_LENGTH + 1];
    Date dateVersion;
    Varchar varcharVersion;
};


//
// Comparison operators
//
enum CompOp {
    NO_OP,                                      // no comparison
    EQ_OP, NE_OP, LT_OP, GT_OP, LE_OP, GE_OP    // binary atomic operators
};

#endif //ROBODBMS_TYPE_H
