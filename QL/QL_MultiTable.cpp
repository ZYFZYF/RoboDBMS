//
// Created by 赵鋆峰 on 2019/12/31.
//

#include "QL_MultiTable.h"
#include "../PS/PS_Expr.h"
#include "../utils/Utils.h"
#include "../utils/PrintError.h"
#include <set>

//groupBy的话要扫描两遍，标记是不是第一遍
extern bool is_first_iteration_round;
//对每个组可能有不同的聚合函数，因此不能直接用group作为最终结果查询的key，选择新增一维aggregationIndex，每次碰到一个新的group中的一次聚合，就aggregationIndex++（其实这样可以省略group）
extern int aggregation_count;
//记录已经计算结束的group，没有group语句的不更新
std::set<std::string> insertGroups;
//TODO: 实际场景的话，该hashTable不一定能在内存里全部存的下（本质上我们需要实现一个基于磁盘的HashMap）
extern std::map<std::pair<std::string, int>, PS_Expr> group_aggregation_expr;

QL_MultiTable::QL_MultiTable(std::vector<TableMeta> *tableMetaList) {
    tableNum = tableMetaList->size();
    tableList.reserve(tableNum);
    recordList.reserve(tableNum);
    int recordNum[tableNum];
    int index[tableNum];
    for (int i = 0; i < tableNum; i++) {
        SM_Table table((*tableMetaList)[i]);
        recordNum[i] = table.count();
        index[i] = i;
    }
    //for (int i = 0; i < tableNum; i++)std::cout << index[i] << ' ' << recordNum[index[i]] << std::endl;
    //std::sort(index, index + tableNum, [&](int x, int y) -> bool { return recordNum[x] > recordNum[y]; });
    //for (int i = 0; i < tableNum; i++)std::cout << index[i] << ' ' << recordNum[index[i]] << std::endl;
    int maxRecordNum = 0;
    for (int i = 0; i < tableNum; i++)maxRecordNum = std::max(maxRecordNum, recordNum[i]);
    long long minCost = 1 << 30;
    int minCostIndex = 0;
    int indexCount = 0;
    //简单估计了，对不同表的遍历顺序对结果的一个影响，最终结论应当是小表在前最好（其实根本不用估计，很直观的结论）
    do {
        long long cost = 0, nowRecord = 1;
        for (int i = 0; i < tableNum; i++) {
            cost += nowRecord;
            // TODO: 这里的估计可能有点问题，就算不到最大的表这儿，nowRecord也可能已经非常大了
            // if (nowRecord != maxRecordNum)nowRecord *= recordNum[index[i]];
            // if (recordNum[index[i]] == maxRecordNum)nowRecord = maxRecordNum;
            nowRecord *= recordNum[index[i]];
        }
        cost += nowRecord;
        if (cost < minCost) {
            minCost = cost;
            minCostIndex = indexCount;
        }
        indexCount++;
        printf("\n表的顺序为 ");
        for (int j = 0; j < tableNum; j++)printf(" %s:%d ", (*tableMetaList)[index[j]].name, recordNum[index[j]]);
        printf("时预计耗费为%lld", cost);
    } while (std::next_permutation(index, index + tableNum));
    for (int i = 0; i < tableNum; i++)index[i] = i;
    for (int i = 0; i < minCostIndex; i++)std::next_permutation(index, index + tableNum);
    for (int i = 0; i < tableNum; i++)
        if (index[i] != i) {
            printf("\n调整表的顺序从 ");
            for (int j = 0; j < tableNum; j++)printf(" %s:%d ", (*tableMetaList)[j].name, recordNum[j]);
            printf(" 到 ");
            for (int j = 0; j < tableNum; j++)printf(" %s:%d ", (*tableMetaList)[index[j]].name, recordNum[index[j]]);
            break;
        }
    for (int i = 0; i < tableNum; i++) {
        tableList.emplace_back((*tableMetaList)[index[i]]);
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
    insertGroups.clear();
    group_aggregation_expr.clear();
    is_first_iteration_round = true;
    isFirstIterate = true;
    aggregation_count = 0;
    DO(iterateTables(0))
    //如果发现了聚合函数，那么需要进行第二遍扫描
    if (aggregation_count > 0 || groupByList != nullptr) {
        insertGroups.clear();
        is_first_iteration_round = false;
        isFirstIterate = true;
        DO(iterateTables(0))
    }

    delete smTable;
    smTable = nullptr;
    auto cost_time = clock() - start_time;
    printf("\n查询: 结果共计%d条，花费%.3f秒", totalCount, (float) cost_time / CLOCKS_PER_SEC);
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

// 目前是用的最简单的方式 Simple/Stupid Nested Loop Join，尽可能用了Index
// TODO: 更好的做法是join的时候两个表排序，然后双指针（或者说类似归并），但排序比较耗时，所以如果两个表在这上面都有B+树索引的话，就更容易做了
// 排序也是不需要的，有hash即可（类似于给第一个表建了一个hash索引，还可以用一个布隆过滤器来加速不存在的键的查找，build + probe） 如果写不到内存里，就先partition到磁盘里，每个分片分别join
RC QL_MultiTable::iterateTables(int n) {
    if (n == tableNum) {
        //获取所属组的信息，用字符串来表示
        std::string group{};
        if (groupByList) {
            for (auto &groupColumn:*groupByList) {
                eval(groupColumn);
                group += groupColumn.to_string();
            }
        } else group = "NULL";
        //第二轮或者没有group by可以插入
        bool canInsert = !is_first_iteration_round || (aggregation_count == 0 && groupByList == nullptr);
        //第一次枚举到头
        if (isFirstIterate) {
            memset(&targetMeta, 0, sizeof(TableMeta));
            targetMeta.tableId = -1;
            targetMeta.columnNum = 0;
            strcpy(targetMeta.name, name.data());
            strcpy(targetMeta.createName, name.data());
            for (auto &value:*valueList) {
                TRY(eval(value, group))
            }
            canInsert = !is_first_iteration_round || (aggregation_count == 0 && groupByList == nullptr);
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
                        //groupby第一次循环的时候会把字符串的最大长度求出来
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
        //每次看情况拿下一次的节点
        //TODO: 这里最好是用迭代器来实现，而不是全加载到内存
        auto ridList = tableList[n].filter(conditionList);
        for (auto &rid : ridList) {
            tableList[n].getRecordFromRID(rid, recordList[n]);
            for (auto &condition: *conditionList) {
                eval(condition, "NULL", n);
            }
            TRY(iterateTables(n + 1))
        }
    }
    return OK_RC;
}

//对该分组group下求表达式value的值
//maxI用来限制只能选择前多少个枚举的表
RC QL_MultiTable::eval(PS_Expr &value, std::string group, int maxI) {
    //常数直接返回
    if (value.isConst)return OK_RC;
    //从里面拿列的值
    if (value.isColumn) {
        auto[i, j] = getColumn(value.tableName, value.columnName);
        if (i > maxI) {
            //如果这列是后面的，那么置为不知道
            value.type = UNKNOWN;
            return OK_RC;
        }
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
    if (value.left)TRY(eval(*value.left, group, maxI))
    if (Utils::isLogic(value.op)) {
        value.type = BOOL;
        //短路操作
        if (value.left && value.left->value.boolValue) {
            value.value.boolValue = true;
            return OK_RC;
        }
    }
    if (value.right)TRY(eval(*value.right, group, maxI))
    TRY(value.pushUp(group))
    return OK_RC;
}

