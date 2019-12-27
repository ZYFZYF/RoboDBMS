//
// Created by 赵鋆峰 on 2019/11/16.
//

#include <cmath>
#include <cstring>
#include "Utils.h"

#define MAX_ROOT_PATH_LENGTH 100

char dataL[MAX_VARCHAR_LENGTH];
char dataR[MAX_VARCHAR_LENGTH];
char rootPath[MAX_ROOT_PATH_LENGTH];

bool Utils::Compare(void *value1, void *value2, AttrType attrType, int attrLength, Operator compOp) {
    switch (compOp) {
        case NO_OP:
            return true;
        case EQ_OP:
            return Cmp(value1, value2, attrType, attrLength) == 0;
        case NE_OP:
            return Cmp(value1, value2, attrType, attrLength) != 0;
        case LT_OP:
            return Cmp(value1, value2, attrType, attrLength) < 0;
        case GT_OP:
            return Cmp(value1, value2, attrType, attrLength) > 0;
        case LE_OP:
            return Cmp(value1, value2, attrType, attrLength) <= 0;
        case GE_OP:
            return Cmp(value1, value2, attrType, attrLength) >= 0;
    }
}

void Utils::GetMinimumValue(void *value, AttrType attrType) {
}

void Utils::GetMaximumValue(void *value, AttrType attrType) {
}

std::string Utils::getIndexFileName(const char *fileName, int indexNo) {
    return std::string(fileName) + "_" + std::to_string(indexNo) + ".index";
}

std::string Utils::getStringPoolFileName(const char *fileName) {
    return std::string(fileName) + ".stringpool";
}

std::string Utils::getRecordFileName(const char *fileName) {
    return std::string(fileName) + ".record";
}

int Utils::Cmp(void *value1, void *value2, AttrType attrType, int attrLength) {
    switch (attrType) {

        case INT:
            return *(int *) value1 - *(int *) value2;
        case FLOAT: {
            float gap = *(float *) value1 - *(float *) value2;
            if (std::fabs(gap) < 1e-5) {
                return 0;
            } else if (gap < 0) {
                return -1;
            } else {
                return 1;
            }
        }
        case STRING:
            return strncmp((char *) value1, (char *) value2, attrLength);
        case DATE: {
            auto date1 = (Date *) value1, date2 = (Date *) value2;
            if (date1->year != date2->year) {
                return date1->year - date2->year;
            } else {
                if (date1->month != date2->month) {
                    return date1->month - date2->month;
                } else return date1->day < date2->day;
            }
        }
        case VARCHAR: {
            auto varchar1 = (Varchar *) value1, varchar2 = (Varchar *) value2;
            varchar1->getData(dataL);
            varchar2->getData(dataR);
            int cmp = memcmp(dataL, dataR, std::min(varchar1->length, varchar2->length));
            if (cmp == 0) {
                return varchar1->length - varchar2->length;
            } else {
                return cmp;
            }
        }
        case ATTRARRAY:
//            printf("%d %s %d %s ", *(int *) ((char *) value1 + ATTR_TYPE_LENGTH + 4),
//                   (char *) value1 + ATTR_TYPE_LENGTH + 8 + ATTR_TYPE_LENGTH + 4,
//                   *(int *) ((char *) value2 + ATTR_TYPE_LENGTH + 4),
//                   (char *) value2 + ATTR_TYPE_LENGTH + 8 + ATTR_TYPE_LENGTH + 4);
            for (int i = 0, length; i < attrLength; i += ATTR_TYPE_LENGTH + 4 + length) {
                AttrType type = *(AttrType *) ((char *) value1 + i);
                length = *(int *) ((char *) value1 + i + ATTR_TYPE_LENGTH);
                int cmp = Cmp((char *) value1 + i + ATTR_TYPE_LENGTH + 4, (char *) value2 + i + ATTR_TYPE_LENGTH + 4,
                              type, length);
                if (cmp != 0) {
                    return cmp;
                }
            }
            return 0;
    }
}

std::string Utils::getAbsolutePath(const char *filename) {
    static char *root = getcwd(rootPath, MAX_ROOT_PATH_LENGTH);
    return std::string(root) + "/" + std::string(filename);
}

bool Utils::isComparable(Operator op) {
    return op == NO_OP || op == EQ_OP || op == NE_OP || op == LT_OP || op == GT_OP || op == LE_OP || op == GE_OP;

}

bool Utils::isArithmetic(Operator op) {
    return op == PLUS_OP || op == MINUS_OP || op == MUL_OP || op == DIV_OP || op == MOD_OP;

}

bool Utils::isLogic(Operator op) {
    return op == NOT_OP || op == AND_OP || op == OR_OP;

}
