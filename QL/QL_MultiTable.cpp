//
// Created by 赵鋆峰 on 2019/12/31.
//

#include "QL_MultiTable.h"
#include <tuple>
#include "../PS/PS_Expr.h"

QL_MultiTable::QL_MultiTable(std::vector<TableMeta> *tableMetaList) {
    tableNum = tableMetaList->size();
    for (auto &tableMeta:*tableMetaList)tableList.emplace_back(SM_Table(tableMeta));
}

TableMeta
QL_MultiTable::select(std::vector<PS_Expr> *_valueList, std::vector<PS_Expr> *_conditionList, std::string &_name) {
    valueList = _valueList;
    conditionList = _conditionList;
    name = _name;
    //先拿到所有遍历的节点
    ridListList.clear();
    for (int i = 0; i < tableNum; i++)ridListList.emplace_back(tableList[i].filter(conditionList));
    recordList.clear();
    isFirstIterate = true;
    iterateTables(0);
    delete smTable;
    return targetMeta;
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

RC QL_MultiTable::iterateTables(int n) {
    if (n == tableNum + 1) {
        //枚举到头了
        if (isFirstIterate) {
            memset(&targetMeta, 0, sizeof(TableMeta));
            targetMeta.tableId = -1;
            targetMeta.columnNum = 0;
            strcpy(targetMeta.name, name.data());
            for (auto &value:*valueList) {
                TRY(eval(value))
                targetMeta.columnNum++;
                //TODO 注意，这样的转换会脱掉一些信息，比如float的位数信息
                targetMeta.columns[targetMeta.columnNum].attrType = value.type;
                switch (value.type) {
                    case INT:
                    case FLOAT: {
                        targetMeta.columns[targetMeta.columnNum].attrLength = 4;
                        break;
                    }
                    case STRING: {
                        targetMeta.columns[targetMeta.columnNum].attrLength = MAX_CHAR_LENGTH;
                        break;
                    }
                    case DATE: {
                        targetMeta.columns[targetMeta.columnNum].attrLength = sizeof(DATE);
                        break;
                    }
                    default:
                        throw "it should not be here";
                }
                strcpy(targetMeta.columns[targetMeta.columnNum].name, value.name.data());
            }
            smTable = new SM_Table(targetMeta);
            isFirstIterate = false;
        }
        char record[smTable->getRecordSize()];
        for (int i = 0; i < targetMeta.columnNum; i++) {
            TRY(eval((*valueList)[i]))
            TRY(smTable->setColumnDataByExpr(record, i, (*valueList)[i]))
        }
        TRY(smTable->insertRecord(record))
    } else {
        for (int i = 0; i < ridListList[n].size(); i++) {
            RM_Record rmRecord;
            tableList[i].getRecordFromRID(ridListList[n][i], rmRecord);
            recordList.emplace_back(rmRecord);
            TRY(iterateTables(n + 1))
            recordList.pop_back();
        }
    }
    return OK_RC;
}

RC QL_MultiTable::eval(PS_Expr &value) {
    return IX_INSERT_TWICE;
}
