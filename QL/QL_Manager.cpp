//
// Created by 赵鋆峰 on 2019/12/2.
//

#include "QL_Manager.h"
#include "../SM/SM_Table.h"
#include "../utils/PrintError.h"
#include "../utils/Utils.h"
#include "QL_MultiTable.h"

QL_Manager &QL_Manager::Instance() {
    static QL_Manager instance;
    return instance;
}

QL_Manager::~QL_Manager() = default;

QL_Manager::QL_Manager() : rmManager(RM_Manager::Instance()), ixManager(IX_Manager::Instance()),
                           smManager(SM_Manager::Instance()) {

}

RC
QL_Manager::Insert(const char *tbName, std::vector<const char *> *columnList, std::vector<AttrValue> *constValueList) {
    TableId tableId = SM_Manager::Instance().GetTableIdFromName(tbName);
    if (tableId < 0)return SM_TABLE_NOT_EXIST;
    SM_Table table(tableId);
    std::vector<ColumnId> columnIdList;
    if (columnList != nullptr) {
        for (auto &column:*columnList) {
            ColumnId columnId = SM_Manager::Instance().GetColumnIdFromName(tableId, column);
            if (columnId < 0)return SM_COLUMN_NOT_EXIST;
            columnIdList.push_back(columnId);
        }
    }
    char record[table.getRecordSize()];
    TRY(table.setRecordData(record, columnList == nullptr ? nullptr : &columnIdList, constValueList));
    TRY(table.insertRecord(record));
    return OK_RC;
}

char line[MAX_LINE_LENGTH];
char *test[100];

RC QL_Manager::Insert(const char *tbName, const char *fileName) {
    auto start_time = clock();
    //这里很奇怪，这里所处的位置是cmake-build-debug/QL下，所以要注意文件的路径书写方式
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == nullptr)return QL_FILE_NOT_EXIST;
    //构造ConstValueList，手动来分割字符串
    //TODO 效率问题，不知道strtok会不会更快
    int totalCount = 0, insertCount = 0;
    while (fgets(line, MAX_LINE_LENGTH, fp) != nullptr) {
        totalCount++;
        auto constValueList = new std::vector<AttrValue>;
        int length = strlen(line);
        for (int l = 0, r; l < length; l = r + 1) {
            r = l;
            while (r < length && line[r] != '|')r++;
            line[r] = '\0';
            if (r < length) {
                AttrValue attrValue{};
                attrValue.charValue = line + l;
                if (strlen(line + l) == 0)attrValue.isNull = true;
                constValueList->emplace_back(attrValue);
            }
        }
        //以行为单位插入，某一行可以不成功
        RC rc;
        if ((rc = Insert(tbName, nullptr, constValueList)) == OK_RC) {
            insertCount++;
        } else {
            printError(rc);
        }
        delete constValueList;
    }
    fclose(fp);
    auto cost_time = clock() - start_time;
    printf("插入: 共计%d条，成功%d条，花费%.3f秒\n", totalCount, insertCount, (float) cost_time / CLOCKS_PER_SEC);
    return OK_RC;
}

RC QL_Manager::Delete(const char *tbName, std::vector<PS_Expr> *conditionList) {
    TableId tableId = SM_Manager::Instance().GetTableIdFromName(tbName);
    if (tableId < 0)return SM_TABLE_NOT_EXIST;
    SM_Table table(tableId);
    TRY(table.deleteWhereConditionSatisfied(conditionList));
    return OK_RC;
}

RC QL_Manager::Count(const char *tbName) {
    TableId tableId = SM_Manager::Instance().GetTableIdFromName(tbName);
    if (tableId < 0)return SM_TABLE_NOT_EXIST;
    SM_Table table(tableId);
    printf("Table %s has %d rows\n", tbName, table.count());
    return OK_RC;
}

RC QL_Manager::Update(const char *tbName, std::vector<std::pair<std::string, PS_Expr> > *assignExprList,
                      std::vector<PS_Expr> *conditionList) {
    TableId tableId = SM_Manager::Instance().GetTableIdFromName(tbName);
    if (tableId < 0)return SM_TABLE_NOT_EXIST;
    SM_Table table(tableId);
    TRY(table.updateWhereConditionSatisfied(assignExprList, conditionList));
    return OK_RC;
}

RC QL_Manager::Select(std::vector<PS_Expr> *valueList, std::vector<TableMeta> *tableMetaList,
                      std::vector<PS_Expr> *conditionList, std::vector<PS_Expr> *groupByList,
                      std::vector<const char *> *orderByColumn, bool increaseOrder, int limitOffset, int limitLength) {
    std::string name = "zyfdsb";
    auto *multiTable = new QL_MultiTable(tableMetaList);
    if (conditionList == nullptr)conditionList = new std::vector<PS_Expr>;
    if (groupByList != nullptr) {
        for (auto &value: *groupByList) {
            if (!value.isColumn) {
                throw "unsupported group by non-column data";
            }
        }
    }

    if (valueList == nullptr) {
        valueList = new std::vector<PS_Expr>;
        //如果没有分组那么就是所有列，否则应该是被分组的所有列
        if (groupByList == nullptr) {
            for (auto &tableMeat: *tableMetaList)
                for (int i = 0; i < tableMeat.columnNum; i++) {
                    valueList->emplace_back(tableMeat.name, tableMeat.columns[i].name);
                }
        } else {
            for (auto &value:*groupByList) {
                valueList->emplace_back(value.tableName.data(), value.columnName.data());
            }
        }
    }
    TableMeta tableMeta = multiTable->select(valueList, conditionList, name, groupByList);
    delete multiTable;
    if (tableMeta.columnNum) {
        auto *table = new SM_Table(tableMeta);
        if (orderByColumn != nullptr) {
            table->orderBy(orderByColumn, increaseOrder, limitOffset, limitLength);
        }
        table->showRecords(100);
        delete table;
        //把这个临时表删掉
        DestroyTable(tableMeta.createName);
    }
    return OK_RC;
}

TableMeta
QL_Manager::getTableFromSelect(const char *name, std::vector<PS_Expr> *valueList, std::vector<TableMeta> *tableMetaList,
                               std::vector<PS_Expr> *conditionList, std::vector<PS_Expr> *groupByList,
                               std::vector<const char *> *orderByColumn, bool increaseOrder,
                               int limitOffset, int limitLength) {
    std::string tableName = name;
    DestroyTable(tableName.data());
    auto *multiTable = new QL_MultiTable(tableMetaList);
    if (conditionList == nullptr)conditionList = new std::vector<PS_Expr>;
    if (groupByList != nullptr) {
        for (auto &value: *groupByList) {
            if (!value.isColumn) {
                throw "unsupported group by non-column data";
            }
        }
    }

    if (valueList == nullptr) {
        valueList = new std::vector<PS_Expr>;
        //如果没有分组那么就是所有列，否则应该是被分组的所有列
        if (groupByList == nullptr) {
            for (auto &tableMeat: *tableMetaList)
                for (int i = 0; i < tableMeat.columnNum; i++) {
                    valueList->emplace_back(tableMeat.name, tableMeat.columns[i].name);
                }
        } else {
            for (auto &value:*groupByList) {
                valueList->emplace_back(value.tableName.data(), value.columnName.data());
            }
        }
    }
    TableMeta tableMeta = multiTable->select(valueList, conditionList, tableName, groupByList);
    delete multiTable;
    auto *table = new SM_Table(tableMeta);
    if (orderByColumn != nullptr) {
        table->orderBy(orderByColumn, increaseOrder, limitOffset, limitLength);
    }
    delete table;
    return tableMeta;
}

void QL_Manager::DestroyTable(const char *tbName) {
    std::string recordFileName = Utils::getRecordFileName(tbName);
    DO(RM_Manager::Instance().DestroyFile(recordFileName.c_str()))
    std::string stringPoolFileName = Utils::getStringPoolFileName(tbName);
    DO(SP_Manager::DestroyStringPool(stringPoolFileName.c_str()))
}

std::vector<PS_Expr> *
QL_Manager::getExprListFromSelect(std::vector<PS_Expr> *valueList, std::vector<TableMeta> *tableMetaList,
                                  std::vector<PS_Expr> *conditionList, std::vector<PS_Expr> *groupByList,
                                  std::vector<const char *> *orderByColumn, bool increaseOrder, int limitOffset,
                                  int limitLength) {
    std::string name = "zyfdsb";
    auto multiTable = new QL_MultiTable(tableMetaList);
    if (conditionList == nullptr)conditionList = new std::vector<PS_Expr>;
    if (groupByList != nullptr) {
        for (auto &value: *groupByList) {
            if (!value.isColumn) {
                throw "unsupported group by non-column data";
            }
        }
    }

    if (valueList == nullptr) {
        valueList = new std::vector<PS_Expr>;
        //如果没有分组那么就是所有列，否则应该是被分组的所有列
        if (groupByList == nullptr) {
            for (auto &tableMeat: *tableMetaList)
                for (int i = 0; i < tableMeat.columnNum; i++) {
                    valueList->emplace_back(tableMeat.name, tableMeat.columns[i].name);
                }
        } else {
            for (auto &value:*groupByList) {
                valueList->emplace_back(value.tableName.data(), value.columnName.data());
            }
        }
    }
    //只能选一列成list
    if (valueList->size() != 1) {
        throw "only support select one column in recursive\n";
    }

    TableMeta tableMeta = multiTable->select(valueList, conditionList, name, groupByList);
    delete multiTable;
    auto *table = new SM_Table(tableMeta);
    if (orderByColumn != nullptr) {
        table->orderBy(orderByColumn, increaseOrder, limitOffset, limitLength);
    }
    auto ret = table->extractValueInRecords();
//    int cnt = 0;
//    for (auto &expr:*ret) {
//        printf("%d %d\n", ++cnt, expr.value.intValue);
//    }
    delete table;
    //把这个临时表删掉
    DestroyTable(tableMeta.createName);
    return ret;
}
