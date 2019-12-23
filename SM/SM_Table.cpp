//
// Created by 赵鋆峰 on 2019/12/21.
//

#include "SM_Table.h"
#include "../utils/Utils.h"
#include "../RM/RM_Manager.h"
#include "../RM/RM_FileScan.h"

SM_Table::SM_Table(const TableMeta &tableMeta) : tableMeta(tableMeta) {
    recordSize = 0;
    for (int i = 0; i < tableMeta.columnNum; i++) {
        columnOffset[i] = recordSize;
        recordSize += tableMeta.columns[i].attrLength + 1;//多一位来存储是否是NULL，放在数据的开头
    }
    std::string recordFileName = Utils::getRecordFileName(tableMeta.createName);
    if (!access(recordFileName.c_str(), F_OK)) {
        RM_Manager::Instance().CreateFile(recordFileName.c_str(), recordSize);
    }
    RM_Manager::Instance().OpenFile(recordFileName.c_str(), rmFileHandle);
}

char *SM_Table::getColumnData(char *record, ColumnId columnId) {
    int offset = columnOffset[columnId];
    //开头第一位存的是isNull
    if (record[offset]) {
        return nullptr;
    } else {
        return record + offset + 1;
    }
}

RC SM_Table::setRecordData(char *record, std::vector<ColumnId> columnIdList, std::vector<AttrValue> *constValueList) {
    return PF_EOF;
}

RC SM_Table::insertRecord(const char *record) {
    RM_RID rmRid;
    TRY(rmFileHandle.InsertRec(record, rmRid));
    return OK_RC;
}

void SM_Table::showRecords(int num) {
    //分割线
    std::string splitLine(tableMeta.columnNum * COLUMN_SHOW_LENGTH, '-');
    std::cout << splitLine << std::endl;
    std::string headerLine;
    for (int i = 0; i < tableMeta.columnNum; i++) {
        std::string columnName = std::string(tableMeta.columns[i].name);
        headerLine.append(columnName);
        if (columnName.length() < COLUMN_SHOW_LENGTH)headerLine.append(COLUMN_SHOW_LENGTH - columnName.length(), ' ');
    }
    std::cout << headerLine << std::endl;
    std::cout << splitLine << std::endl;
    //打开一个无条件遍历
    RM_FileScan rmFileScan;
    rmFileScan.OpenScan(rmFileHandle, INT, 0, 0, NO_OP, nullptr);
    RM_Record rmRecord;
    //如果输出够了条数或者没有了就停止
    for (int i = 0; (i < num || num == -1) && (rmFileScan.GetNextRec(rmRecord) == OK_RC); i++) {
        char *record;
        rmRecord.GetData(record);
        std::cout << formatRecordToString(record) << std::endl;;
    }
    std::cout << splitLine << std::endl;;
}

std::string SM_Table::formatRecordToString(char *record) {
    std::string line;
    for (int i = 0; i < tableMeta.columnNum; i++) {
        std::string column;
        char *data = getColumnData(record, i);
        if (data == nullptr)column = "NULL";
        else {
            switch (tableMeta.columns[i].attrType) {
                case INT: {
                    column = std::to_string(*(int *) data);
                    break;
                }
                case FLOAT: {

                    char temp[100];
                    char format[100];
                    sprintf(format, "%%%d.%df", tableMeta.columns[i].integerLength, tableMeta.columns->decimalLength);
                    sprintf(temp, format, *(float *) data);
                    column = temp;
                    break;
                }
                case DATE: {
                    Date date = *(Date *) data;
                    column = std::to_string(date.year) + "-" + std::to_string(date.month) + "-" +
                             std::to_string(date.day);
                    break;
                }
                case STRING: {
                    column = data;
                    break;
                }

                case VARCHAR: {
                    char temp[tableMeta.columns[i].stringMaxLength];
                    ((Varchar *) data)->getData(temp);
                    column = temp;
                    break;
                }
                case ATTRARRAY:
                    //不应该在这儿
                    exit(0);
                    break;
            }
        }
        if (column.length() < COLUMN_SHOW_LENGTH)column.append(COLUMN_SHOW_LENGTH - column.length(), ' ');
        line.append(column);
    }
    return line;
}

