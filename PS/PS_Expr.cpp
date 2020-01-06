//
// Created by 赵鋆峰 on 2019/12/27.
//

#include <csignal>
#include "PS_Expr.h"
#include "../SM/SM_Manager.h"
#include "../utils/Utils.h"
#include "../SM/SM_Table.h"

PS_Expr::PS_Expr() {
    isConst = true;
    type = UNKNOWN;
    value.isNull = true;
}

PS_Expr::PS_Expr(bool _value) {
    isConst = true;
    type = BOOL;
    value.boolValue = _value;
}

PS_Expr::PS_Expr(int _value) {
    isConst = true;
    type = INT;
    value.intValue = _value;
}

PS_Expr::PS_Expr(float _value) {
    isConst = true;
    type = FLOAT;
    value.floatValue = _value;
}

PS_Expr::PS_Expr(char *_value) {
    isConst = true;
    type = STRING;
    string = std::string(_value);
    stringMaxLength = string.size();
}

PS_Expr::PS_Expr(char *tbName, char *colName) {
    isColumn = true;
    type = UNKNOWN;
    if (tbName == nullptr)tableName = "";
    else tableName = std::string(tbName);
    columnName = std::string(colName);
}

RC PS_Expr::eval(SM_Table &table, char *record) {
    //常数直接返回
    if (isConst)return OK_RC;
    //从里面拿列的值
    if (isColumn) {
        //TODO 这里只考虑从当前表的列里面获取值，暂且不考虑多表
        for (int i = 0; i < table.tableMeta.columnNum; i++)
            if (strcmp(columnName.c_str(), table.tableMeta.columns[i].name) == 0) {
                type = table.tableMeta.columns[i].attrType;
                char *data = table.getColumnData(record, i);
                if (data == nullptr) {
                    value.isNull = true;
                } else {
                    value.isNull = false;
                    switch (type) {
                        case INT: {
                            value.intValue = *(int *) data;
                            break;
                        }
                        case FLOAT: {
                            value.floatValue = *(float *) data;
                            break;
                        }
                        case STRING: {
                            string = std::string(data);
                            break;
                        }
                        case DATE: {
                            value.dateValue = *(Date *) data;
                            break;
                        }
                        case VARCHAR: {
                            char temp[table.tableMeta.columns[i].stringMaxLength];
                            ((Varchar *) data)->getData(temp);
                            string = temp;
                            break;
                        }
                    }
                }
                //在计算时VARCHAR和STRING一视同仁，并且都转成std::string进行操作
                if (type == VARCHAR)type = STRING;
                return OK_RC;
            }
        return SM_COLUMN_NOT_EXIST;
    }
    if (left)TRY(left->eval(table, record))
    if (Utils::isLogic(op)) {
        type = BOOL;
        //短路操作
        if (left && left->value.boolValue) {
            value.boolValue = true;
            return OK_RC;
        }
    }
    if (right)TRY(right->eval(table, record))
    TRY(pushUp())
    return OK_RC;
}

PS_Expr::PS_Expr(PS_Expr *_left, Operator _op, PS_Expr *_right) {
    op = _op;
    left = _left;
    right = _right;
    if (left && left->type != UNKNOWN && right->type != UNKNOWN)pushUp();
}

RC PS_Expr::pushUp() {
    if (left && left->isConst && right && right->isConst)isConst = true;
    switch (op) {
        case EQ_OP: {
            type = BOOL;
            if (left->value.isNull && right->value.isNull) {
                value.boolValue = true;
                return OK_RC;
            }
            if (left->value.isNull != right->value.isNull) {
                value.boolValue = false;
                return OK_RC;
            }
            value.boolValue = isComparable() && cmp() == 0;
            break;
        }
        case NE_OP: {
            type = BOOL;
            //printf("%s %d\n", left->string.c_str(), left->value.isNull);
            if (left->value.isNull && right->value.isNull) {
                value.boolValue = false;
                return OK_RC;
            }
            if (left->value.isNull != right->value.isNull) {
                value.boolValue = true;
                return OK_RC;
            }
            value.boolValue = isComparable() && cmp() == 0;
            break;
        }
        case LT_OP: {
            type = BOOL;
            if (left->value.isNull or right->value.isNull) {
                value.boolValue = false;
                return OK_RC;
            }
            value.boolValue = isComparable() && cmp() < 0;
            break;
        }
        case GT_OP: {
            type = BOOL;
            if (left->value.isNull or right->value.isNull) {
                value.boolValue = false;
                return OK_RC;
            }
            value.boolValue = isComparable() && cmp() > 0;
            break;
        }
        case LE_OP: {
            type = BOOL;
            if (left->value.isNull or right->value.isNull) {
                value.boolValue = false;
                return OK_RC;
            }
            value.boolValue = isComparable() && cmp() <= 0;
            break;
        }
        case GE_OP: {
            type = BOOL;
            if (left->value.isNull or right->value.isNull) {
                value.boolValue = false;
                return OK_RC;
            }
            value.boolValue = isComparable() && cmp() >= 0;
            break;
        }
        case PLUS_OP: {
            if (left->type == INT && right->type == INT) {
                type = INT;
                value.intValue = left->value.intValue + right->value.intValue;
            } else if (left->type == INT && right->type == FLOAT) {
                type = FLOAT;
                value.floatValue = float(left->value.intValue) + right->value.floatValue;
            } else if (left->type == FLOAT && right->type == INT) {
                type = FLOAT;
                value.floatValue = left->value.floatValue + float(right->value.intValue);
            } else if (left->type == FLOAT && right->type == FLOAT) {
                type = FLOAT;
                value.floatValue = left->value.floatValue + right->value.floatValue;
            } else if (left->type == STRING && right->type == STRING) {
                type = STRING;
                string = left->string + right->string;
                stringMaxLength = left->stringMaxLength + right->stringMaxLength;
            } else return QL_UNSUPPORTED_OPERATION_TYPE;
            break;
        }
        case MINUS_OP: {
            if (left->type == INT && right->type == INT) {
                type = INT;
                value.intValue = left->value.intValue - right->value.intValue;
            } else if (left->type == INT && right->type == FLOAT) {
                type = FLOAT;
                value.floatValue = float(left->value.intValue) - right->value.floatValue;
            } else if (left->type == FLOAT && right->type == INT) {
                type = FLOAT;
                value.floatValue = left->value.floatValue - float(right->value.intValue);
            } else if (left->type == FLOAT && right->type == FLOAT) {
                type = FLOAT;
                value.floatValue = left->value.floatValue - right->value.floatValue;
            } else return QL_UNSUPPORTED_OPERATION_TYPE;
            break;
        }
        case MUL_OP: {
            if (left->type == INT && right->type == INT) {
                type = INT;
                value.intValue = left->value.intValue * right->value.intValue;
            } else if (left->type == INT && right->type == FLOAT) {
                type = FLOAT;
                value.floatValue = float(left->value.intValue) * right->value.floatValue;
            } else if (left->type == FLOAT && right->type == INT) {
                type = FLOAT;
                value.floatValue = left->value.floatValue * float(right->value.intValue);
            } else if (left->type == FLOAT && right->type == FLOAT) {
                type = FLOAT;
                value.floatValue = left->value.floatValue * right->value.floatValue;
            } else return QL_UNSUPPORTED_OPERATION_TYPE;
            break;
        }
        case DIV_OP: {
            if (left->type == INT && right->type == INT) {
                type = INT;
                value.intValue = left->value.intValue / right->value.intValue;
            } else if (left->type == INT && right->type == FLOAT) {
                type = FLOAT;
                value.floatValue = float(left->value.intValue) / right->value.floatValue;
            } else if (left->type == FLOAT && right->type == INT) {
                type = FLOAT;
                value.floatValue = left->value.floatValue / float(right->value.intValue);
            } else if (left->type == FLOAT && right->type == FLOAT) {
                type = FLOAT;
                value.floatValue = left->value.floatValue / right->value.floatValue;
            } else return QL_UNSUPPORTED_OPERATION_TYPE;
            break;
        }
        case MOD_OP: {
            if (left->type == INT && right->type == INT) {
                type = INT;
                value.intValue = left->value.intValue % right->value.intValue;
            } else return QL_UNSUPPORTED_OPERATION_TYPE;
            break;
        }
        case NOT_OP: {
            type = BOOL;
            value.boolValue = !right->value.boolValue;
            break;
        }
        case OR_OP: {
            type = BOOL;
            value.boolValue = left->value.boolValue || right->value.boolValue;
            break;
        }
        default:
            return QL_UNSUPPORTED_OPERATION_TYPE;
    }
    return OK_RC;
}

int PS_Expr::cmp() {
    switch (left->type) {
        case BOOL: {
            return int(left->value.boolValue) - int(right->value.boolValue);
        }
        case INT: {
            if (right->type == INT) {
                return left->value.intValue - right->value.intValue;
            } else if (right->type == FLOAT) {
                return Utils::transferFloatToCmpInt(left->value.intValue - right->value.floatValue);
            }
        }
        case FLOAT: {
            if (right->type == INT) {
                return Utils::transferFloatToCmpInt(left->value.floatValue - right->value.intValue);
            } else if (right->type == FLOAT) {
                return Utils::transferFloatToCmpInt(left->value.floatValue - right->value.floatValue);
            }
        }
        case STRING: {
            if (right->type == DATE) {
                Date &date1 = left->value.dateValue, &date2 = right->value.dateValue;
                Utils::transferStringToDate(left->string.c_str(), date1);
                if (date1.year != date2.year) {
                    return date1.year - date2.year;
                } else {
                    if (date1.month != date2.month) {
                        return date1.month - date2.month;
                    } else return date1.day < date2.day;
                }
            } else if (right->type == STRING)
                return strcmp(left->string.c_str(), right->string.c_str());
        }
        case DATE: {
            Date &date1 = left->value.dateValue, &date2 = right->value.dateValue;
            if (right->type == STRING) {
                Utils::transferStringToDate(right->string.c_str(), date2);
            }
            if (date1.year != date2.year) {
                return date1.year - date2.year;
            } else {
                if (date1.month != date2.month) {
                    return date1.month - date2.month;
                } else return date1.day - date2.day;
            }
        }
    }
}

bool PS_Expr::isComparable() {
    //普通的可比较函数
    AttrType type1 = left->type, type2 = right->type;
    if (type1 == type2 || ((type1 == INT || type1 == FLOAT) && (type2 == INT || type2 == FLOAT)))return true;
    //字符串和日期的比较
    if (type1 == STRING && type2 == DATE) {
        Date date{};
        return Utils::transferStringToDate(left->string.c_str(), date) == OK_RC;
    }
    if (type1 == DATE && type2 == STRING) {
        Date date{};
        return Utils::transferStringToDate(right->string.c_str(), date) == OK_RC;
    }
    return false;
}

void PS_Expr::setName(const std::string &_name) {
    name = _name;
}
