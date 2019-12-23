//
// Created by 赵鋆峰 on 2019/12/21.
//

#include <cmath>
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
    if (access(recordFileName.c_str(), F_OK) < 0) {
        RM_Manager::Instance().CreateFile(recordFileName.c_str(), recordSize);
    }
    RM_Manager::Instance().OpenFile(recordFileName.c_str(), rmFileHandle);
    std::string stringPoolFileName = Utils::getStringPoolFileName(tableMeta.createName);
    if (access(stringPoolFileName.c_str(), F_OK) < 0) {
        SP_Manager::CreateStringPool(stringPoolFileName.c_str());
    }
    SP_Manager::OpenStringPool(stringPoolFileName.c_str(), spHandle);
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

RC SM_Table::setRecordData(char *record, std::vector<ColumnId> *columnIdList, std::vector<AttrValue> *constValueList) {
    //列为空则代表是默认的顺序排列
    if (columnIdList == nullptr) {
        //检测给定参数个数是否与行数相同
        if (constValueList->size() != tableMeta.columnNum)return QL_COLUMNS_VALUES_DONT_MATCH;
        for (int i = 0; i < tableMeta.columnNum; i++) {
            TRY(setColumnData(record, i, (*constValueList)[i]));
        }
    } else {
        //检测给定参数个数是否与行数相同
        if (columnIdList->size() != constValueList->size())return QL_COLUMNS_VALUES_DONT_MATCH;
        bool hasValue[tableMeta.columnNum];
        for (int i = 0; i < tableMeta.columnNum; i++)hasValue[i] = false;
        for (int i = 0; i < columnIdList->size(); i++) {
            hasValue[(*columnIdList)[i]] = true;
            TRY(setColumnData(record, (*columnIdList)[i], (*constValueList)[i]));
        }
        for (int i = 0; i < tableMeta.columnNum; i++)
            if (!hasValue[i]) {
                TRY(setColumnNull(record, i));
            }
    }
    return OK_RC;
}

RC SM_Table::insertRecord(const char *record) {
    RM_RID rmRid;
    //TODO 插入之前要进行逻辑判断，主键是否重复、外键是否存在
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
    rmFileScan.CloseScan();
    std::cout << splitLine << std::endl;;
}

std::string SM_Table::formatRecordToString(char *record) {
    std::string line;
    for (int i = 0; i < tableMeta.columnNum; i++) {
        std::string column;
        char *data = getColumnData(record, i);
        if (data == nullptr)column = "NULL";
        else column = formatColumnToString(i, data);
        if (column.length() < COLUMN_SHOW_LENGTH)column.append(COLUMN_SHOW_LENGTH - column.length(), ' ');
        line.append(column);
    }
    return line;
}

RC SM_Table::setColumnData(char *record, ColumnId columnId, AttrValue attrValue) {
    //命令里有传该参数的值，但也有可能是null
    if (attrValue.isNull) {
        TRY(setColumnNull(record, columnId))
    }
    //先设为不是null
    record[columnOffset[columnId]] = 0;
    char *data = record + columnOffset[columnId] + 1;
    TRY(completeAttrValueByColumnId(columnId, attrValue));
    switch (tableMeta.columns[columnId].attrType) {
        case INT: {
            *(int *) data = attrValue.intValue;
            break;
        }
        case FLOAT: {
            *(float *) data = attrValue.floatValue;
            break;
        }
        case STRING: {
            strcpy(data, attrValue.charValue);
            break;
        }
        case DATE: {
            *(Date *) data = attrValue.dateValue;
            break;
        }
        case VARCHAR: {
            *(Varchar *) data = attrValue.varcharValue;
            break;
        }
        case ATTRARRAY: { //不应该在这儿
            exit(0);
            break;
        }
    }
    return OK_RC;
}

RC SM_Table::setColumnNull(char *record, ColumnId columnId) {
    //命令里没传，所以认为是null，但要先看是不是有defaultValue
    if (tableMeta.columns[columnId].hasDefaultValue) {
        TRY(setColumnData(record, columnId, tableMeta.columns[columnId].defaultValue))
        return OK_RC;
    }
    if (tableMeta.columns[columnId].allowNull) {
        record[columnOffset[columnId]] = 1;
        return OK_RC;
    }
    return QL_COLUMN_NOT_ALLOW_NULL;
}

std::string SM_Table::formatColumnToString(ColumnId columnId, char *data) {
    std::string column;
    switch (tableMeta.columns[columnId].attrType) {
        case INT: {
            column = std::to_string(*(int *) data);
            break;
        }
        case FLOAT: {

            char temp[100];
            char format[100];
            if (tableMeta.columns[columnId].integerLength) {
                sprintf(format, "%%%d.%df", tableMeta.columns[columnId].integerLength,
                        tableMeta.columns->decimalLength);
                sprintf(temp, format, *(float *) data);
                column = temp;
            } else {
                column = std::to_string(*(float *) data);
            }
            break;
        }
        case DATE: {
            Date date = *(Date *) data;
            char temp[100];
            sprintf(temp, "%04d-%02d-%02d", date.year, date.month, date.day);
            column = temp;
            break;
        }
        case STRING: {
            column = data;
            break;
        }

        case VARCHAR: {
            char temp[tableMeta.columns[columnId].stringMaxLength];
            ((Varchar *) data)->getData(temp);
            column = temp;
            break;
        }
        case ATTRARRAY:
            //不应该在这儿
            exit(0);
            break;
    }
    return column;
}

RC SM_Table::completeAttrValueByColumnId(ColumnId columnId, AttrValue &attrValue) {
    switch (tableMeta.columns[columnId].attrType) {
        case INT: {
            char *endPtr;
            int num = strtol(attrValue.charValue, &endPtr, 10);
            if (errno == ERANGE)return QL_INT_OUT_OF_RANGE;
            printf("%s\n", attrValue.charValue);
            if (strlen(endPtr) != 0)return QL_INT_CONT_CONVERT_TO_INT;
            attrValue.intValue = num;
            break;
        }
        case FLOAT: {
            char *endPtr;
            float num = strtof(attrValue.charValue, &endPtr);
            if (errno == ERANGE)return QL_FLOAT_OUT_OF_RANGE;
            if (strlen(endPtr) != 0)return QL_FLOAT_CONT_CONVERT_TO_FLOAT;
            int integerLength = tableMeta.columns[columnId].integerLength;
            int decimalLength = tableMeta.columns[columnId].decimalLength;
            if (integerLength < 0 || decimalLength < 0 || decimalLength > integerLength)return QL_DECIMAL_FORMAT_ERROR;
            if (integerLength > 0 && fabsf(num) >= powf(10, integerLength))return QL_FLOAT_OUT_OF_RANGE;
            attrValue.floatValue = num;
            break;
        }
        case STRING: {
            if (strlen(attrValue.charValue) >= tableMeta.columns[columnId].attrLength)return QL_CHAR_TOO_LONG;
            strcpy(attrValue.stringValue, attrValue.charValue);
            break;
        }
        case DATE: {
            char *endPtr;
            Date date{};
            date.year = strtol(attrValue.charValue, &endPtr, 10);
            if (endPtr != attrValue.charValue + 4 || endPtr[0] != '-')return QL_DATE_CONT_CONVERT_TO_DATE;
            date.month = strtol(attrValue.charValue + 5, &endPtr, 10);
            if (endPtr != attrValue.charValue + 7 || endPtr[0] != '-')return QL_DATE_CONT_CONVERT_TO_DATE;
            date.day = strtol(attrValue.charValue + 8, &endPtr, 10);
            if (endPtr != attrValue.charValue + 10 || strlen(endPtr) != 0)return QL_DATE_CONT_CONVERT_TO_DATE;
            if (!date.isValid())return QL_DATE_IS_NOT_VALID;
            attrValue.dateValue = date;
            break;
        }
        case VARCHAR: {
            Varchar varchar{};
            varchar.length = strlen(attrValue.charValue);
            strcpy(varchar.spName, Utils::getStringPoolFileName(tableMeta.createName).c_str());
            TRY(spHandle.InsertString(attrValue.charValue, varchar.length, varchar.offset))
            attrValue.varcharValue = varchar;
            break;
        }
        case ATTRARRAY: { //不应该在这儿
            exit(0);
            break;
        }
    }
    return OK_RC;
}

std::string SM_Table::formatAttrValueToString(ColumnId columnId, AttrValue attrValue) {
    switch (tableMeta.columns[columnId].attrType) {
        case INT:
            return formatColumnToString(columnId, (char *) &(attrValue.intValue));
        case FLOAT:
            return formatColumnToString(columnId, (char *) &(attrValue.floatValue));
        case STRING:
            return formatColumnToString(columnId, (char *) &(attrValue.stringValue));

        case DATE:
            return formatColumnToString(columnId, (char *) &(attrValue.dateValue));

        case VARCHAR:
            return formatColumnToString(columnId, (char *) &(attrValue.varcharValue));

        case ATTRARRAY:
            //不应该在这儿
            break;
    }
}

int SM_Table::getRecordSize() const {
    return recordSize;
}

SM_Table::~SM_Table() {
    RM_Manager::Instance().CloseFile(rmFileHandle);
    SP_Manager::CloseStringPool(spHandle);
}
