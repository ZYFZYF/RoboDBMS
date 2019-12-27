//
// Created by 赵鋆峰 on 2019/12/27.
//

#include "PS_Expr.h"
#include "../SM/SM_Manager.h"

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
}

PS_Expr::PS_Expr(char *tbName, char *colName) {
    isColumn = true;
    type = UNKNOWN;
    if (tbName == nullptr)tableName = "";
    else tableName = std::string(tbName);
    columnName = std::string(colName);
}

RC PS_Expr::eval(SM_Table &table, char *record) {
    if (isConst)return OK_RC;
    //TODO 根据计算类型来进行eval，注意短路操作的实现
    return OK_RC;
}

PS_Expr::PS_Expr(PS_Expr *_left, Operator _op, PS_Expr *_right) {
    op = _op;
    left = _left;
    right = _right;
    //TODO 根据左右表达式来确定一些属性值
}
