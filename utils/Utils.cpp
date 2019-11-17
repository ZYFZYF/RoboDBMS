//
// Created by 赵鋆峰 on 2019/11/16.
//

#include <cstring>
#include "Utils.h"

bool Utils::Compare(void *value1, void *value2, AttrType attrType, int attrLength, CompOp compOp) {
    switch (attrType) {
        case INT: {
            switch (compOp) {
                case NO_OP:
                    return true;
                case EQ_OP:
                    return *(int *) value1 == *(int *) value2;
                case NE_OP:
                    return *(int *) value1 != *(int *) value2;
                case LT_OP:
                    return *(int *) value1 < *(int *) value2;
                case LE_OP:
                    return *(int *) value1 <= *(int *) value2;
                case GT_OP:
                    return *(int *) value1 > *(int *) value2;
                case GE_OP:
                    return *(int *) value1 >= *(int *) value2;
            }
        }
        case FLOAT: {
            switch (compOp) {
                case NO_OP:
                    return true;
                case EQ_OP:
                    return *(float *) value1 == *(float *) value2;
                case NE_OP:
                    return *(float *) value1 != *(float *) value2;
                case LT_OP:
                    return *(float *) value1 < *(float *) value2;
                case LE_OP:
                    return *(float *) value1 <= *(float *) value2;
                case GT_OP:
                    return *(float *) value1 > *(float *) value2;
                case GE_OP:
                    return *(float *) value1 >= *(float *) value2;
            }
        }
        case STRING: {
            switch (compOp) {
                case NO_OP:
                    return true;
                case EQ_OP:
                    return strncmp((char *) value1, (char *) value2, attrLength) == 0;
                case NE_OP:
                    return strncmp((char *) value1, (char *) value2, attrLength) != 0;
                case LT_OP:
                    return strncmp((char *) value1, (char *) value2, attrLength) < 0;
                case LE_OP:
                    return strncmp((char *) value1, (char *) value2, attrLength) <= 0;
                case GT_OP:
                    return strncmp((char *) value1, (char *) value2, attrLength) > 0;
                case GE_OP:
                    return strncmp((char *) value1, (char *) value2, attrLength) >= 0;
            }
        }
        case DATE: {
            //TODO  实现Date类型的支持
            return true;
        }
        case VARCHAR: {
            //TODO  实现Varchar类型的比较函数，可能需要把SP的Handle传进来
            return true;
        }
    }
}

void Utils::GetMinimumValue(void *value, AttrType attrType) {
}

void Utils::GetMaximumValue(void *value, AttrType attrType) {
}
