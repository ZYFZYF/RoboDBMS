//
// Created by 赵鋆峰 on 2019/12/2.
//

#include "QL_Manager.h"
#include "../SM/SM_Table.h"
#include "../utils/PrintError.h"

QL_Manager &QL_Manager::Instance() {
    static QL_Manager instance;
    return instance;
}

QL_Manager::~QL_Manager() = default;

RC QL_Manager::Select() {
    return PF_EOF;
}

RC QL_Manager::Update() {
    return PF_EOF;
}

QL_Manager::QL_Manager() : rmManager(RM_Manager::Instance()), ixManager(IX_Manager::Instance()),
                           smManager(SM_Manager::Instance()) {

}

RC
QL_Manager::Insert(const char *tbName, std::vector<const char *> *columnList, std::vector<AttrValue> *constValueList) {
    TableId tableId = SM_Manager::Instance().GetTableIdFromName(tbName);
    if (tableId < 0)return SM_TABLE_NOT_EXIST;
    SM_Table table(SM_Manager::Instance().GetTableMeta(tableId));
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
    //这里很奇怪，这里所处的位置是cmake-build-debug/QL下，所以要注意文件的路径书写方式
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == nullptr)return QL_FILE_NOT_EXIST;
    //构造ConstValueList，手动来分割字符串
    //TODO 效率问题，不知道strtok会不会更快
    while (fgets(line, MAX_LINE_LENGTH, fp) != nullptr) {
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
        DO(Insert(tbName, nullptr, constValueList))
        delete constValueList;
    }
    fclose(fp);
    return OK_RC;
}

RC QL_Manager::Delete(const char *tbName, PS_Expr *condition) {
    TableId tableId = SM_Manager::Instance().GetTableIdFromName(tbName);
    if (tableId < 0)return SM_TABLE_NOT_EXIST;
    SM_Table table(SM_Manager::Instance().GetTableMeta(tableId));
    TRY(table.deleteCondition(condition));
    return OK_RC;
}
