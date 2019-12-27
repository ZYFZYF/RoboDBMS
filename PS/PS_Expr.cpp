//
// Created by 赵鋆峰 on 2019/12/27.
//

#include "PS_Expr.h"
#include "../SM/SM_Manager.h"

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
    tableId = SM_Manager::Instance().GetTableIdFromName(tbName);
    columnId = SM_Manager::Instance().GetColumnIdFromName(tableId, colName);
    type = SM_Manager::Instance().GetType(tableId, columnId);
}

RC PS_Expr::eval(SM_Table &table, char *record) {
    if (isConst)return OK_RC;
    //TODO 根据计算类型来进行eval，注意短路操作的实现
    return OK_RC;
}
