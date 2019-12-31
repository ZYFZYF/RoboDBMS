//
// Created by 赵鋆峰 on 2019/12/31.
//

#include "QL_MultiTable.h"
#include <tuple>

QL_MultiTable::QL_MultiTable(std::vector<TableMeta> *tableMetaList) {
    tableNum = tableMetaList->size();
    for (auto &tableMeta:*tableMetaList)tableList.emplace_back(SM_Table(tableMeta));
}

TableMeta QL_MultiTable::select(std::vector<PS_Expr> *valueList, std::vector<PS_Expr> *conditionList) {
    return TableMeta();
}

std::pair<SM_Table *, ColumnId> QL_MultiTable::getColumn(std::string &tbName, std::string &columnName) {
    if (tbName.length() > 0) {
        for (auto &table:tableList)
            if (strcmp(table.tableMeta.name, tbName.data()) == 0) {
                for (int i = 0; i < MAX_COLUMN_NUM; i++)
                    if (strcmp(table.tableMeta.columns[i].name, columnName.data()) == 0) {
                        return std::make_pair(&table, i);
                    }
            }
        throw "Can't find " + tbName + "." + columnName;
    } else {
        int match = 0;
        for (auto &table:tableList)
            for (auto &column : table.tableMeta.columns)
                if (strcmp(column.name, columnName.data()) == 0) {
                    match++;
                }
        if (match == 0) {
            throw "Can't find " + columnName;
        } else if (match > 1) {
            throw "There are multi " + columnName;
        } else {
            for (auto &table:tableList)
                for (int i = 0; i < MAX_COLUMN_NUM; i++)
                    if (strcmp(table.tableMeta.columns[i].name, columnName.data()) == 0) {
                        return std::make_pair(&table, i);
                    }
        }
    }
}
