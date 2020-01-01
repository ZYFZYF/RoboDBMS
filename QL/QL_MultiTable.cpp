//
// Created by 赵鋆峰 on 2019/12/31.
//

#include "QL_MultiTable.h"
#include "../PS/PS_Expr.h"
#include "../utils/Utils.h"

QL_MultiTable::QL_MultiTable(std::vector<TableMeta> *tableMetaList) {
    tableNum = tableMetaList->size();
    for (auto &tableMeta:*tableMetaList) {
        tableList.emplace_back(tableMeta);
        recordList.emplace_back();
    }
}

TableMeta
QL_MultiTable::select(std::vector<PS_Expr> *_valueList, std::vector<PS_Expr> *_conditionList, std::string &_name) {
    valueList = _valueList;
    conditionList = _conditionList;
    name = _name;
    //先拿到所有遍历的节点
    ridListList.clear();
    for (int i = 0; i < tableNum; i++) {
        ridListList.push_back(tableList[i].filter(conditionList));
    }
    isFirstIterate = true;
    iterateTables(0);
    delete smTable;
    return targetMeta;
}

std::pair<int, ColumnId> QL_MultiTable::getColumn(std::string &tbName, std::string &columnName) {
    if (tbName.length() > 0) {
        for (int i = 0; i < tableNum; i++) {
            SM_Table &table = tableList[i];
            if (strcmp(table.tableMeta.name, tbName.data()) == 0) {
                for (int j = 0; j < MAX_COLUMN_NUM; j++)
                    if (strcmp(table.tableMeta.columns[j].name, columnName.data()) == 0) {
                        return std::make_pair(i, j);
                    }
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
            for (int i = 0; i < tableNum; i++) {
                SM_Table &table = tableList[i];
                for (int j = 0; j < MAX_COLUMN_NUM; j++)
                    if (strcmp(table.tableMeta.columns[j].name, columnName.data()) == 0) {
                        return std::make_pair(i, j);
                    }
            }
        }
    }
}

RC QL_MultiTable::iterateTables(int n) {
    if (n == tableNum) {
        //枚举到头了
        if (isFirstIterate) {
            memset(&targetMeta, 0, sizeof(TableMeta));
            targetMeta.tableId = -1;
            targetMeta.columnNum = 0;
            strcpy(targetMeta.name, name.data());
            strcpy(targetMeta.createName, name.data());
            for (auto &value:*valueList) {
                TRY(eval(value))
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
                targetMeta.columnNum++;
            }
            smTable = new SM_Table(targetMeta);
            isFirstIterate = false;
        }
        bool conditionSatisfied = true;
        for (auto &condition:*conditionList) {
            TRY(eval(condition))
            conditionSatisfied &= condition.value.boolValue;
            if (!conditionSatisfied)break;
        }
        if (conditionSatisfied) {
            char record[smTable->getRecordSize()];
            for (int i = 0; i < targetMeta.columnNum; i++) {
                TRY(eval((*valueList)[i]))
                TRY(smTable->setColumnDataByExpr(record + smTable->columnOffset[i], i, (*valueList)[i], true))
            }
            TRY(smTable->insertRecord(record))
        }
    } else {
        for (int i = 0; i < ridListList[n].size(); i++) {
            tableList[n].getRecordFromRID(ridListList[n][i], recordList[n]);
            TRY(iterateTables(n + 1))
        }
    }
    return OK_RC;
}

RC QL_MultiTable::eval(PS_Expr &value) {
    //常数直接返回
    if (value.isConst)return OK_RC;
    //从里面拿列的值
    if (value.isColumn) {
        //TODO 这里只考虑从当前表的列里面获取值，暂且不考虑多表
        auto[i, j] = getColumn(value.tableName, value.columnName);
//        printf("tableName = %s columnName = %s get tableIndex = %d columnsId = %d\n", value.tableName.data(),
//               value.columnName.data(),
//               i, j);
        if (value.name.empty())value.name = tableList[i].tableMeta.columns[j].name;
        value.type = tableList[i].tableMeta.columns[j].attrType;
        char *data = tableList[i].getColumnData(recordList[i].getData(), j);
        int z = *(int *) data;
        if (data == nullptr) {
            value.value.isNull = true;
        } else {
            value.value.isNull = false;
            switch (value.type) {
                case INT: {
                    value.value.intValue = *(int *) data;
                    break;
                }
                case FLOAT: {
                    value.value.floatValue = *(float *) data;
                    break;
                }
                case STRING: {
                    value.string = std::string(data);
                    break;
                }
                case DATE: {
                    value.value.dateValue = *(Date *) data;
                    break;
                }
                case VARCHAR: {
                    char temp[tableList[i].tableMeta.columns[j].stringMaxLength];
                    ((Varchar *) data)->getData(temp);
                    value.string = temp;
                    break;
                }
            }
        }
        //在计算时VARCHAR和STRING一视同仁，并且都转成std::string进行操作
        if (value.type == VARCHAR)value.type = STRING;
        return OK_RC;
    }
    if (value.left)TRY(eval(*value.left))
    if (Utils::isLogic(value.op)) {
        value.type = BOOL;
        //短路操作
        if (value.left && value.left->value.boolValue) {
            value.value.boolValue = true;
            return OK_RC;
        }
    }
    if (value.right)TRY(eval(*value.right))
    TRY(value.pushUp())
    return OK_RC;
}

