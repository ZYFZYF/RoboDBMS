//
// Created by 赵鋆峰 on 2019/12/31.
//

#ifndef ROBODBMS_QL_MULTITABLE_H
#define ROBODBMS_QL_MULTITABLE_H


#include "../SM/SM_Table.h"

class QL_MultiTable {
    explicit QL_MultiTable(std::vector<TableMeta> *tableMetaList);

    TableMeta select(std::vector<PS_Expr> *valueList, std::vector<PS_Expr> *conditionList);

private:
    int tableNum;
    std::vector<SM_Table> tableList;

};


#endif //ROBODBMS_QL_MULTITABLE_H
