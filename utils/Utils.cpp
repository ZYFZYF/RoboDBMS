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
            switch (compOp) {
                case NO_OP:
                    return true;
                case EQ_OP:
                    return *(Date *) value1 == *(Date *) value2;
                case NE_OP:
                    return *(Date *) value1 != *(Date *) value2;
                case LT_OP:
                    return *(Date *) value1 < *(Date *) value2;
                case LE_OP:
                    return *(Date *) value1 <= *(Date *) value2;
                case GT_OP:
                    return *(Date *) value1 > *(Date *) value2;
                case GE_OP:
                    return *(Date *) value1 >= *(Date *) value2;
            }
        }
        case VARCHAR: {
            switch (compOp) {
                case NO_OP:
                    return true;
                case EQ_OP:
                    return *(Varchar *) value1 == *(Varchar *) value2;
                case NE_OP:
                    return *(Varchar *) value1 != *(Varchar *) value2;
                case LT_OP:
                    return *(Varchar *) value1 < *(Varchar *) value2;
                case LE_OP:
                    return *(Varchar *) value1 <= *(Varchar *) value2;
                case GT_OP:
                    return *(Varchar *) value1 > *(Varchar *) value2;
                case GE_OP:
                    return *(Varchar *) value1 >= *(Varchar *) value2;
            }
        }
        case ATTRARRAY:
            break;
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
