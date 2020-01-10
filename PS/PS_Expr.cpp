//
// Created by 赵鋆峰 on 2019/12/27.
//

#include <csignal>
#include "PS_Expr.h"
#include "../SM/SM_Manager.h"
#include "../utils/Utils.h"
#include "../SM/SM_Table.h"

//第一次循环用来计算聚合函数的值
bool is_first_iteration = true;
int aggregation_count = 0;
std::map<std::pair<std::string, int>, PS_Expr> group_aggregation_expr{};


PS_Expr::PS_Expr() {
    isConst = true;
    type = INT;
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

PS_Expr::PS_Expr(std::vector<PS_Expr> *_exprList) {
    isColumn = false;
    isConst = true;
    type = ATTRARRAY;
    exprList = _exprList;
}

PS_Expr::PS_Expr(PS_Expr *_left, Operator _op, bool _isAny, std::vector<PS_Expr> *_exprList) {
    isConst = false;
    isColumn = false;
    left = _left;
    op = _op;
    type = UNKNOWN;
    isAny = _isAny;
    exprList = _exprList;
    right = nullptr;
    pushUp();
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
        //有一种情况是这是其他表的column
        return OK_RC;
        //return SM_COLUMN_NOT_EXIST;
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
    //聚合函数不pushup，否则可能会计算aggregation_cnt
    if (op == MAX_OP || op == MIN_OP || op == AVG_OP || op == SUM_OP || op == COUNT_OP)return;
    pushUp();
}

RC PS_Expr::pushUp(std::string group) {
    //如果没得更新立马返回，或者右边没有计算完毕
    if (right && right->type == UNKNOWN) return OK_RC;
    if (left && left->type == UNKNOWN) return OK_RC;
    if (left && left->isConst && right && right->isConst)isConst = true;
    //这是嵌套子查询
    if (exprList != nullptr) {
        type = BOOL;
        int cnt = 0;
        for (auto &expr:*exprList) {
            right = &expr;
            switch (op) {
                case EQ_OP: {
                    if (left->value.isNull && right->value.isNull) {
                        cnt++;
                    } else if (left->value.isNull != right->value.isNull) {
                    } else
                        cnt += isComparable() && cmp() == 0;
                    break;
                }
                case NE_OP: {
                    if (left->value.isNull && right->value.isNull) {
                    }
                    if (left->value.isNull != right->value.isNull) {
                        cnt++;
                    } else
                        cnt += isComparable() && cmp() == 0;
                    break;
                }
                case LT_OP: {
                    if (left->value.isNull or right->value.isNull) {
                    } else cnt += isComparable() && cmp() < 0;
                    break;
                }
                case GT_OP: {
                    if (left->value.isNull or right->value.isNull) {
                    } else cnt += isComparable() && cmp() > 0;
                    break;
                }
                case LE_OP: {
                    if (left->value.isNull or right->value.isNull) {
                    } else cnt += isComparable() && cmp() <= 0;
                    break;
                }
                case GE_OP: {
                    if (left->value.isNull or right->value.isNull) {
                    } else cnt += value.boolValue = isComparable() && cmp() >= 0;
                    break;
                }
                case LIKE_OP: {
                    if (left->value.isNull or right->value.isNull) {
                    }
                    if (right->type == STRING) {
                        if (left->type == DATE) {
                            std::string temp = Utils::transferDateToString(left->value.dateValue);
                            cnt += Utils::like(temp, right->string);
                        }
                        if (left->type == STRING) {
                            cnt += Utils::like(left->string, right->string);
                        }
                    }
                    break;
                }
                default:
                    throw "unsupported compare with a array";
            }
        }
        right = nullptr;
        value.boolValue = (isAny && cnt) || (!isAny && cnt == exprList->size());
    } else {
        //下面是正常的
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
            case LIKE_OP: {
                type = BOOL;
                if (left->value.isNull or right->value.isNull) {
                    value.boolValue = false;
                    return OK_RC;
                }
                value.boolValue = false;
                if (right->type == STRING) {
                    if (left->type == DATE) {
                        std::string temp = Utils::transferDateToString(left->value.dateValue);
                        value.boolValue = Utils::like(temp, right->string);
                    }
                    if (left->type == STRING) {
                        value.boolValue = Utils::like(left->string, right->string);
                    }
                }
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
            case MAX_OP:
            case MIN_OP:
            case SUM_OP:
            case AVG_OP:
            case COUNT_OP: {
                //第一次的时候算到这儿卡住，上面的计算就不要了
                if (is_first_iteration) {
                    type = UNKNOWN;
                    if (aggregationIndex == 0)aggregationIndex = ++aggregation_count;
                    auto key = std::make_pair(group, aggregationIndex);
                    //头一次的话用来init
                    if (group_aggregation_expr.find(key) == group_aggregation_expr.end()) {
                        if (!right->value.isNull) {
                            group_aggregation_expr[key] = PS_Expr();
                            group_aggregation_expr[key].initAggregation(op, right);
                        }
                    } else {
                        group_aggregation_expr[key].updateAggregation(op, right);
                    }
                } else {
                    //第二次的时候就给定type，可以往上规约
                    type = right->type;
                    auto key = std::make_pair(group, aggregationIndex);
                    auto expr = group_aggregation_expr[key];
                    //std::cout << group << std::endl;
                    value = expr.value;
                    string = expr.string;
                    stringMaxLength = expr.stringMaxLength;
                    //只有这一种特殊情况需要改type
                    if (op == AVG_OP && type == INT) type = FLOAT;
                    //还有一种情况_(:з」∠)_
                    if (op == COUNT_OP)type = INT;
                }
                break;
            }
            case IN_OP: {
                type = BOOL;
                value.boolValue = false;
                for (auto &expr:*right->exprList) {
                    if (left->type == expr.type || (left->type == DATE && right->type == STRING)) {
                        switch (left->type) {
                            case INT: {
                                value.boolValue |= (left->value.intValue == expr.value.intValue);
                                break;
                            }
                            case FLOAT: {
                                value.boolValue |= (left->value.floatValue == expr.value.floatValue);
                                break;
                            }
                            case STRING: {
                                value.boolValue |= (left->string == expr.string);
                                break;
                            }
                            case DATE: {
                                value.boolValue |= (Utils::transferDateToString(left->value.dateValue) ==
                                                    expr.string);
                                break;
                            }
                            default:
                                break;
                        }
                    }
                }
                break;
            }
            case NIN_OP: {
                type = BOOL;
                value.boolValue = true;
                for (auto &expr:*right->exprList) {
                    if (left->type == expr.type || (left->type == DATE && right->type == STRING)) {
                        switch (left->type) {
                            case INT: {
                                value.boolValue &= (left->value.intValue != expr.value.intValue);
                                break;
                            }
                            case FLOAT: {
                                value.boolValue &= (left->value.floatValue != expr.value.floatValue);
                                break;
                            }
                            case STRING: {
                                value.boolValue &= (left->string != expr.string);
                                break;
                            }
                            case DATE: {
                                value.boolValue &= !(Utils::transferDateToString(left->value.dateValue) ==
                                                     expr.string);
                                break;
                            }
                            default:
                                break;
                        }
                    }
                }
                break;
            }
            default:
                return QL_UNSUPPORTED_OPERATION_TYPE;
        }
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
        default:
            throw "unsupported cmp type";
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

//avg的话直接变成float
RC PS_Expr::initAggregation(Operator op, PS_Expr *expr) {
    value.isNull = false;
    updateCount = 1;
    switch (op) {
        case MAX_OP:
        case MIN_OP: {
            if (expr->type != INT && expr->type != FLOAT && expr->type != STRING && expr->type != DATE)
                throw "Not supported aggregation column";
            value = expr->value;
            string = expr->string;
            break;
        }
        case AVG_OP:
        case SUM_OP: {
            if (expr->type == INT)value.floatValue = expr->value.intValue;
            else if (expr->type == FLOAT)value.floatValue = expr->value.floatValue;
            else
                throw "Not supported aggregation column";
            break;
        }
        case COUNT_OP: {
            value.intValue = 1;
            break;
        }
        default:
            throw "Not aggregation function";
    }
    stringMaxLength = expr->stringMaxLength;
    return OK_RC;
}

RC PS_Expr::updateAggregation(Operator op, PS_Expr *expr) {
    if (expr->value.isNull)return OK_RC;
    switch (op) {
        case MAX_OP: {
            switch (expr->type) {
                case INT: {
                    value.intValue = std::max(value.intValue, expr->value.intValue);
                    break;
                }
                case FLOAT: {
                    value.floatValue = std::max(value.floatValue, expr->value.floatValue);
                    break;
                }
                case STRING: {
                    string = std::max(string, expr->string);
                    break;
                }
                case DATE: {
                    if (value.dateValue < expr->value.dateValue) {
                        value.dateValue = expr->value.dateValue;
                    }
                    break;
                }
            }
            break;
        }
        case MIN_OP: {
            switch (expr->type) {
                case INT: {
                    value.intValue = std::min(value.intValue, expr->value.intValue);
                    break;
                }
                case FLOAT: {
                    value.floatValue = std::min(value.floatValue, expr->value.floatValue);
                    break;
                }
                case STRING: {
                    string = std::min(string, expr->string);
                    break;
                }
                case DATE: {
                    if (expr->value.dateValue < value.dateValue) {
                        value.dateValue = expr->value.dateValue;
                    }
                    break;
                }
            }
            break;
        }
        case AVG_OP: {
            switch (expr->type) {
                case INT: {
                    value.floatValue = (value.floatValue * updateCount + expr->value.intValue) / (updateCount + 1);
                    break;
                }
                case FLOAT: {
                    value.floatValue =
                            (value.floatValue * updateCount + expr->value.floatValue) / (updateCount + 1);
                    break;
                }
            }
            break;
        }
        case SUM_OP: {
            switch (expr->type) {
                case INT: {
                    value.intValue += expr->value.intValue;
                    break;
                }
                case FLOAT: {
                    value.floatValue += expr->value.floatValue;
                    break;
                }
            }
            break;
        }
        case COUNT_OP: {
            value.intValue++;
            break;
        }
        default:
            throw "Not aggregation function";
    }
    updateCount++;
    return OK_RC;
}

std::string PS_Expr::to_string() {
    switch (type) {
        case INT:
            return std::to_string(value.intValue);
        case FLOAT:
            return std::to_string(value.floatValue);
        case STRING:
            return string;
        case DATE:
            return std::to_string(value.dateValue.year) + '-' + std::to_string(value.dateValue.month) + '-' +
                   std::to_string(value.dateValue.day);
        default:
            throw "not supported to string";
    }
}