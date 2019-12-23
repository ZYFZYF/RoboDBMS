//
// Created by 赵鋆峰 on 2019/12/2.
//

#include "QL_Manager.h"
#include "../SM/SM_Table.h"

QL_Manager &QL_Manager::Instance() {
    static QL_Manager instance;
    return instance;
}

QL_Manager::~QL_Manager() {

}

RC QL_Manager::Select() {
    return PF_EOF;
}

RC QL_Manager::Delete() {
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
