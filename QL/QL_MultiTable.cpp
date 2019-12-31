//
// Created by 赵鋆峰 on 2019/12/31.
//

#include "QL_MultiTable.h"

QL_MultiTable::QL_MultiTable(std::vector<TableMeta> *tableMetaList) {
    tableNum = tableMetaList->size();
    for (auto &tableMeta:*tableMetaList)tableList.emplace_back(SM_Table(tableMeta));
}

TableMeta QL_MultiTable::select(std::vector<PS_Expr> *valueList, std::vector<PS_Expr> *conditionList) {
    return TableMeta();
}
