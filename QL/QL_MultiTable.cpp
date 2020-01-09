//
// Created by 赵鋆峰 on 2019/12/31.
//

#include "QL_MultiTable.h"
#include "../PS/PS_Expr.h"
#include "../utils/Utils.h"
#include <set>

extern bool is_first_iteration;
extern int aggregation_count;
std::set<std::string> insertGroups;
extern std::map<std::pair<std::string, int>, PS_Expr> group_aggregation_expr;

QL_MultiTable::QL_MultiTable(std::vector<TableMeta> *tableMetaList) {
    tableNum = tableMetaList->size();
    tableList.reserve(tableNum);
    recordList.reserve(tableNum);
    for (auto &tableMeta:*tableMetaList) {
        tableList.emplace_back(tableMeta);
        recordList.emplace_back();
    }
}

TableMeta
QL_MultiTable::select(std::vector<PS_Expr> *_valueList, std::vector<PS_Expr> *_conditionList, std::string &_name,
                      std::vector<PS_Expr> *_groupByList) {
    auto start_time = clock();
    totalCount = 0;
    valueList = _valueList;
    conditionList = _conditionList;
    groupByList = _groupByList;
    name = _name;
    //先拿到所有遍历的节点
    ridListList.clear();
    for (int i = 0; i < tableNum; i++)ridListList.emplace_back(tableList[i].filter(conditionList));
    group_aggregation_expr.clear();
    is_first_iteration = true;
    isFirstIterate = true;
    aggregation_count = 0;
    iterateTables(0);
    //如果发现了聚合函数，那么需要进行第二遍扫描
    if (aggregation_count > 0 || groupByList != nullptr) {
        insertGroups.clear();
        is_first_iteration = false;
        isFirstIterate = true;
        iterateTables(0);
    }
    delete smTable;
    auto cost_time = clock() - start_time;
    printf("查询: 结果共计%d条，花费%.3f秒\n", totalCount, (float) cost_time / CLOCKS_PER_SEC);
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
        //获取所属组的信息
        std::string group{};
        if (groupByList) {
            for (auto &groupColumn:*groupByList) {
                eval(groupColumn);
                group += groupColumn.to_string();
            }
        } else group = "NULL";

        bool canInsert = !is_first_iteration || (aggregation_count == 0 && groupByList == nullptr);
        //枚举到头了
        if (isFirstIterate) {
            memset(&targetMeta, 0, sizeof(TableMeta));
            targetMeta.tableId = -1;
            targetMeta.columnNum = 0;
            strcpy(targetMeta.name, name.data());
            strcpy(targetMeta.createName, name.data());
            for (auto &value:*valueList) {
                TRY(eval(value, group))
            }
            canInsert = !is_first_iteration || (aggregation_count == 0 && groupByList == nullptr);
            if (canInsert) {
                for (auto &value:*valueList) {
                    //TODO 注意，这样的转换会脱掉一些信息，比如float的位数信息
                    targetMeta.columns[targetMeta.columnNum].attrType = value.type;
                    targetMeta.columns[targetMeta.columnNum].allowNull = true;
                    switch (value.type) {
                        case INT:
                        case FLOAT: {
                            targetMeta.columns[targetMeta.columnNum].attrLength = 4;
                            break;
                        }
                        case STRING: {
                            targetMeta.columns[targetMeta.columnNum].attrLength = value.stringMaxLength;
                            break;
                        }
                        case DATE: {
                            targetMeta.columns[targetMeta.columnNum].attrLength = sizeof(Date);
                            break;
                        }
                        default:
                            throw "it should not be here";
                    }
                    strcpy(targetMeta.columns[targetMeta.columnNum].name, value.name.data());
                    targetMeta.columnNum++;
                }
                smTable = new SM_Table(targetMeta);
            }
        }
        //如果这个group的数据已经被插入了，那么就不再计算等等值
        if (insertGroups.find(group) != insertGroups.end())return OK_RC;
        bool conditionSatisfied = true;
        for (auto &condition:*conditionList) {
            TRY(eval(condition, group))
            conditionSatisfied &= condition.value.boolValue;
            if (!conditionSatisfied)break;
        }
        if (conditionSatisfied) {
            if (!isFirstIterate)
                for (auto &value:*valueList) {
                    TRY(eval(value, group))
                }
            if (canInsert) {
                char record[smTable->getRecordSize()];
                for (int i = 0; i < (*valueList).size(); i++) {
                    TRY(smTable->setColumnDataByExpr(record + smTable->columnOffset[i], i, (*valueList)[i], true))
                }
                totalCount++;
                if (aggregation_count > 0 || groupByList != nullptr)
                    insertGroups.insert(group);//如果有聚合的话，每个group只插入一次，否则插入多次
                TRY(smTable->insertRecord(record))
            }
        }
        isFirstIterate = false;
    } else {
        for (int i = 0; i < ridListList[n].size(); i++) {
            tableList[n].getRecordFromRID(ridListList[n][i], recordList[n]);
            TRY(iterateTables(n + 1))
        }
    }
    return OK_RC;
}

RC QL_MultiTable::eval(PS_Expr &value, std::string group) {
    //常数直接返回
    if (value.isConst)return OK_RC;
    //从里面拿列的值
    if (value.isColumn) {
        auto[i, j] = getColumn(value.tableName, value.columnName);
        if (value.name.empty())value.name = tableList[i].tableMeta.columns[j].name;
        value.type = tableList[i].tableMeta.columns[j].attrType;
        char *data = tableList[i].getColumnData(recordList[i].getData(), j);
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
                    value.stringMaxLength = tableList[i].tableMeta.columns[j].attrLength;
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
                    value.stringMaxLength = tableList[i].tableMeta.columns[j].stringMaxLength;
                    break;
                }
            }
        }
        //在计算时VARCHAR和STRING一视同仁，并且都转成std::string进行操作
        if (value.type == VARCHAR)value.type = STRING;
        return OK_RC;
    }
    if (value.left)TRY(eval(*value.left, group))
    if (Utils::isLogic(value.op)) {
        value.type = BOOL;
        //短路操作
        if (value.left && value.left->value.boolValue) {
            value.value.boolValue = true;
            return OK_RC;
        }
    }
    if (value.right)TRY(eval(*value.right, group))
    TRY(value.pushUp(group))
    return OK_RC;
}

