//
// Created by 赵鋆峰 on 2019/12/21.
//

#include <cmath>
#include "SM_Table.h"
#include "../utils/Utils.h"
#include "../RM/RM_Manager.h"
#include "../RM/RM_FileScan.h"
#include "../IX/IX_Manager.h"
#include "../IX/IX_IndexScan.h"
#include "SM_Manager.h"

SM_Table::SM_Table(TableId _tableId) : tableId(_tableId), tableMeta(SM_Manager::Instance().GetTableMeta(tableId)) {
    init();
}

SM_Table::SM_Table(TableMeta &_tableMeta) : tableId(-1), tableMeta(_tableMeta) {
    init();
}

void SM_Table::init() {
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

RC SM_Table::insertRecord(char *record, bool influencePrimaryKey) {
    RM_RID rmRid;
    //插入之前判断主键是否重复，没有主键不用判断
    if (influencePrimaryKey && tableMeta.primaryKey.keyNum) {
        int indexNo = tableMeta.primaryKey.indexIndex;
        IndexDesc &indexDesc = tableMeta.indexes[indexNo];
        int indexKeyLength = getIndexKeyLength(indexDesc);
        char key[indexKeyLength];
        TRY(composeIndexKey(record, indexDesc, key))
        if (getIndexKeyDuplicateNum(key, indexNo, indexDesc))return QL_PRIMARY_KEY_DUPLICATE;
    }
    //TODO 插入之前判断外键是否存在
    TRY(rmFileHandle.InsertRec(record, rmRid))
    //插入索引
    for (int i = 0; i < MAX_INDEX_NUM; i++) {
        IndexDesc &index = tableMeta.indexes[i];
        if (index.keyNum) {
            char keyData[getIndexKeyLength(index)];
            TRY(composeIndexKey(record, index, keyData));
            IX_IndexHandle ixIndexHandle;
            TRY(IX_Manager::Instance().OpenIndex(tableMeta.createName, i, ixIndexHandle))
            TRY(ixIndexHandle.InsertEntry(keyData, rmRid))
            TRY(IX_Manager::Instance().CloseIndex(ixIndexHandle));
        }
    }
    return OK_RC;
}

void SM_Table::showRecords(int num) {
    //分割线
    std::string splitLine((tableMeta.columnNum + 1) * COLUMN_SHOW_LENGTH, '-');
    std::cout << splitLine << std::endl;
    std::string headerLine;
    headerLine.append("num");
    headerLine.append(COLUMN_SHOW_LENGTH - 3, ' ');
    for (int i = 0; i < tableMeta.columnNum; i++) {
        std::string columnName = std::string(tableMeta.columns[i].name);
        headerLine.append(columnName);
        if (columnName.length() < COLUMN_SHOW_LENGTH)
            headerLine.append(COLUMN_SHOW_LENGTH - columnName.length(), ' ');
    }
    std::cout << headerLine << std::endl;
    std::cout << splitLine << std::endl;
    //打开一个无条件遍历
    RM_FileScan rmFileScan;
    rmFileScan.OpenScan(rmFileHandle);
    RM_Record rmRecord;
    //如果输出够了条数或者没有了就停止
    for (int i = 0; (i < num || num == -1) && (rmFileScan.GetNextRec(rmRecord) == OK_RC); i++) {
        char *record;
        rmRecord.GetData(record);
        std::string number = std::to_string(i + 1);
        number.append(COLUMN_SHOW_LENGTH - number.length(), ' ');
        std::cout << number << formatRecordToString(record) << std::endl;;
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

RC SM_Table::setColumnData(char *record, ColumnId columnId, AttrValue attrValue, bool alreadyComplete) {
    //命令里有传该参数的值，但也有可能是null
    if (attrValue.isNull) {
        TRY(setColumnNull(record, columnId))
        return OK_RC;
    }
    //先设为不是null
    record[columnOffset[columnId]] = 0;
    char *data = record + columnOffset[columnId] + 1;
    if (!alreadyComplete)TRY(completeAttrValueByColumnId(columnId, attrValue));
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
    }
    return OK_RC;
}

RC SM_Table::setColumnNull(char *record, ColumnId columnId) {
    //命令里没传，所以认为是null，但要先看是不是有defaultValue
    if (tableMeta.columns[columnId].hasDefaultValue) {
        TRY(setColumnData(record, columnId, tableMeta.columns[columnId].defaultValue, true))
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
                        tableMeta.columns[columnId].decimalLength);
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
    }
    return column;
}

RC SM_Table::completeAttrValueByColumnId(ColumnId columnId, AttrValue &attrValue) {
    switch (tableMeta.columns[columnId].attrType) {
        case INT: {
            char *endPtr;
            int num = strtol(attrValue.charValue, &endPtr, 10);
            if (errno == ERANGE)return QL_INT_OUT_OF_RANGE;
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
            if (integerLength < 0 || decimalLength < 0 || decimalLength > integerLength)
                return QL_DECIMAL_FORMAT_ERROR;
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
            Date date{};
            TRY(Utils::transferStringToDate(attrValue.charValue, date))
            attrValue.dateValue = date;
            break;
        }
        case VARCHAR: {
            Varchar varchar{};
            varchar.length = strlen(attrValue.charValue);
            if (varchar.length >= tableMeta.columns[columnId].stringMaxLength)return QL_VARCHAR_TOO_LONG;
            strcpy(varchar.spName, Utils::getStringPoolFileName(tableMeta.createName).c_str());
            TRY(spHandle.InsertString(attrValue.charValue, varchar.length, varchar.offset))
            attrValue.varcharValue = varchar;
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
    }
}

int SM_Table::getRecordSize() const {
    return recordSize;
}

SM_Table::~SM_Table() {
    RM_Manager::Instance().CloseFile(rmFileHandle);
    SP_Manager::CloseStringPool(spHandle);
}

RC SM_Table::createIndex(int indexNo, IndexDesc indexDesc, bool allowDuplicate) {
    //自己拉的屎自己擦屁股，保证新建的时候没有，新建的时候出问题的话自己关掉/删掉相关文件
    int attrLength = getIndexKeyLength(indexDesc);
    std::string indexFileName = Utils::getIndexFileName(tableMeta.createName, indexNo);
    TRY(IX_Manager::Instance().CreateIndex(tableMeta.createName, indexNo, indexDesc.keyNum > 1 ? ATTRARRAY
                                                                                               : tableMeta.columns[indexDesc.columnId[0]].attrType,
                                           attrLength))
    IX_IndexHandle ixIndexHandle;
    TRY(IX_Manager::Instance().OpenIndex(tableMeta.createName, indexNo, ixIndexHandle))
    //打开一个无条件遍历
    RM_FileScan rmFileScan;
    TRY(rmFileScan.OpenScan(rmFileHandle))
    RM_Record rmRecord;
    char key[attrLength];
    //逐条组织成index所需要的key，然后insert进去
    while (rmFileScan.GetNextRec(rmRecord) == OK_RC) {
        RM_RID rmRid;
        char *record;
        TRY(rmRecord.GetData(record))
        composeIndexKey(record, indexDesc, key);
        TRY(rmRecord.GetRid(rmRid))
        TRY(ixIndexHandle.InsertEntry(key, rmRid))
    }
    TRY(rmFileScan.CloseScan())
    TRY(IX_Manager::Instance().CloseIndex(ixIndexHandle))
    //先全部插进去，关闭索引的句柄，再来判断unique，因为边插入边查询好像会出问题（感觉是同时握有一个文件的两个句柄导致的？）
    if (!allowDuplicate) {
        TRY(rmFileScan.OpenScan(rmFileHandle))
        while (rmFileScan.GetNextRec(rmRecord) == OK_RC) {
            RM_RID rmRid;
            char *record;
            TRY(rmRecord.GetData(record))
            composeIndexKey(record, indexDesc, key);
            TRY(rmRecord.GetRid(rmRid))
            if (getIndexKeyDuplicateNum(key, indexNo, indexDesc) > 1) {
                TRY(rmFileScan.CloseScan())
                TRY(IX_Manager::Instance().DestroyIndex(tableMeta.createName, indexNo))
                return SM_INDEX_NOT_ALLOW_DUPLICATE;
            }
        }
        TRY(rmFileScan.CloseScan())
    }
    return OK_RC;
}

int SM_Table::getIndexKeyLength(IndexDesc indexDesc) {
    int length = 0;
    for (int i = 0; i < indexDesc.keyNum; i++) {
        length += ATTR_TYPE_LENGTH + 4 + tableMeta.columns[indexDesc.columnId[i]].attrLength;
    }
    //如果只有一列的话直接存就可以了
    if (indexDesc.keyNum == 1)length -= ATTR_TYPE_LENGTH + 4;
    return length;
}

RC SM_Table::composeIndexKey(char *record, IndexDesc indexDesc, char *key) {
    if (indexDesc.keyNum == 1) {
        ColumnId columnId = indexDesc.columnId[0];
        char *data = getColumnData(record, columnId);
        if (data == nullptr)return SM_INDEX_COLUMN_NOT_ALLOW_NULL;
        memcpy(key, data, tableMeta.columns[columnId].attrLength);
        return OK_RC;
    } else {
        //联合索引的key组成形式为attrType1 + attrLength1 + attrValue1 + attrType2 + attrLength2 + attrValue2.....
        int length = 0;
        for (int i = 0; i < indexDesc.keyNum; i++) {
            ColumnId columnId = indexDesc.columnId[i];
            *(AttrType *) (key + length) = tableMeta.columns[columnId].attrType;
            *(int *) (key + length + ATTR_TYPE_LENGTH) = tableMeta.columns[columnId].attrLength;
            char *data = getColumnData(record, columnId);
            if (data == nullptr)return SM_INDEX_COLUMN_NOT_ALLOW_NULL;
            memcpy(key + length + ATTR_TYPE_LENGTH + 4, data, tableMeta.columns[columnId].attrLength);
            length += tableMeta.columns[columnId].attrLength + ATTR_TYPE_LENGTH + 4;
        }
        return OK_RC;
    }
}

int SM_Table::getIndexKeyDuplicateNum(char *key, int indexNo, IndexDesc indexDesc) {
    IX_IndexHandle ixIndexHandle;
    IX_Manager::Instance().OpenIndex(tableMeta.createName, indexNo, ixIndexHandle);
    IX_IndexScan ixIndexScan;
    ixIndexScan.OpenScan(ixIndexHandle, EQ_OP, key);
    int cnt = 0;
    RM_RID rmRid;
    while (ixIndexScan.GetNextEntry(rmRid) == OK_RC) cnt++;
    ixIndexScan.CloseScan();
    IX_Manager::Instance().CloseIndex(ixIndexHandle);
    return cnt;
}

RC SM_Table::deleteWhereConditionSatisfied(std::vector<PS_Expr> *conditionList) {
    //打开一个无条件遍历
    RM_FileScan rmFileScan;
    TRY(rmFileScan.OpenScan(rmFileHandle))
    RM_Record rmRecord;
    while (rmFileScan.GetNextRec(rmRecord) == OK_RC) {
        char *record;
        rmRecord.GetData(record);
        RM_RID rmRid;
        rmRecord.GetRid(rmRid);
        //用这行的值去计算表达式的值
        bool conditionSatisfied = true;
        for (auto &condition:*conditionList) {
            TRY(condition.eval(*this, record))
            conditionSatisfied &= condition.value.boolValue;
            if (!conditionSatisfied)break;
        }
        if (conditionSatisfied) TRY(deleteRecord(record, rmRid))
    }
    TRY(rmFileScan.CloseScan())
    return OK_RC;
}

RC SM_Table::deleteRecord(char *record, const RM_RID &rmRid, bool influencePrimaryKey) {
    //删除之前判断主键的值是否被外联，外联的话不能删除，没有主键不用判断
    if (tableMeta.primaryKey.keyNum) {
        int indexNo = tableMeta.primaryKey.indexIndex;
        IndexDesc &indexDesc = tableMeta.indexes[indexNo];
        int indexKeyLength = getIndexKeyLength(indexDesc);
        char key[indexKeyLength];
        TRY(composeIndexKey(record, indexDesc, key))
        if (influencePrimaryKey) {
            //TODO 判断这个主键是否外链了存在的外键
        }
    }
    TRY(rmFileHandle.DeleteRec(rmRid))
    //删除索引
    for (int i = 0; i < MAX_INDEX_NUM; i++) {
        IndexDesc &index = tableMeta.indexes[i];
        if (index.keyNum) {
            char keyData[getIndexKeyLength(index)];
            TRY(composeIndexKey(record, index, keyData));
            IX_IndexHandle ixIndexHandle;
            TRY(IX_Manager::Instance().OpenIndex(tableMeta.createName, i, ixIndexHandle))
            TRY(ixIndexHandle.DeleteEntry(keyData, rmRid))
            TRY(IX_Manager::Instance().CloseIndex(ixIndexHandle));
        }
    }
    return OK_RC;
}

int SM_Table::count() {
    RM_FileScan rmFileScan;
    rmFileScan.OpenScan(rmFileHandle);
    RM_Record rmRecord;
    int cnt = 0;
    while (rmFileScan.GetNextRec(rmRecord) == OK_RC) {
        cnt++;
    }
    rmFileScan.CloseScan();
    return cnt;
}

RC SM_Table::updateWhereConditionSatisfied(std::vector<std::pair<std::string, PS_Expr> > *assignExprList,
                                           std::vector<PS_Expr> *conditionList) {
    bool influencePrimaryKey = false;
    std::vector<ColumnId> updateColumnIdList;
    for (auto &assignExpr: *assignExprList) {
        ColumnId columnId = SM_Manager::Instance().GetColumnIdFromName(tableId, assignExpr.first.data());
        if (columnId < 0) return SM_COLUMN_NOT_EXIST;
        for (int i = 0; i < tableMeta.primaryKey.keyNum; i++)
            if (tableMeta.primaryKey.columnId[i] == columnId)influencePrimaryKey = true;
        updateColumnIdList.push_back(columnId);
    }
    RM_FileScan rmFileScan;
    TRY(rmFileScan.OpenScan(rmFileHandle))
    RM_Record rmRecord;
    char newRecord[recordSize];
    while (rmFileScan.GetNextRec(rmRecord) == OK_RC) {
        char *record;
        rmRecord.GetData(record);
        RM_RID rmRid;
        rmRecord.GetRid(rmRid);
        bool conditionSatisfied = true;
        for (auto &condition:*conditionList) {
            TRY(condition.eval(*this, record))
            conditionSatisfied &= condition.value.boolValue;
            if (!conditionSatisfied)break;
        }
        if (conditionSatisfied) {
            //拷贝一个新的出来，因为不能直接修改原来的值
            memcpy(newRecord, record, recordSize);
            //这里一定要先尝试赋值，因为先删再赋值有可能出错这样就凭白少了一条记录，如果更新不成功跳过这一条的
            bool updateSuccess = true;
            for (int i = 0; i < updateColumnIdList.size(); i++) {
                if (setColumnDataByExpr(newRecord, updateColumnIdList[i], (*assignExprList)[i].second) !=
                    OK_RC) {
                    updateSuccess = false;
                    break;
                }
            }
            if (updateSuccess) {
                //这些基本不会出错
                TRY(deleteRecord(record, rmRid, influencePrimaryKey))
                TRY(insertRecord(newRecord, influencePrimaryKey));
            }
        }
    }
    TRY(rmFileScan.CloseScan())
    return OK_RC;
}

RC SM_Table::setColumnDataByExpr(char *record, ColumnId columnId, PS_Expr &expr) {
    TRY(expr.eval(*this, record))
    //命令里有传该参数的值，但也有可能是null
    if (expr.value.isNull) {
        TRY(setColumnNull(record, columnId))
        return OK_RC;
    }
    //先设为不是null
    record[columnOffset[columnId]] = 0;
    char *data = record + columnOffset[columnId] + 1;
    switch (tableMeta.columns[columnId].attrType) {
        case INT: {
            if (expr.type == INT)*(int *) data = expr.value.intValue;
            else return QL_UNSUPPORTED_ASSIGN_TYPE;
            break;
        }
        case FLOAT: {
            if (expr.type == FLOAT)*(float *) data = expr.value.floatValue;
            else if (expr.type == INT) *(float *) data = float(expr.value.intValue);
            else return QL_UNSUPPORTED_ASSIGN_TYPE;
            break;
        }
        case STRING: {
            if (expr.type == STRING) {
                if (expr.string.length() >= tableMeta.columns[columnId].attrLength)return QL_CHAR_TOO_LONG;
                strcpy(data, expr.string.data());
            } else return QL_UNSUPPORTED_ASSIGN_TYPE;
            break;
        }
        case DATE: {
            if (expr.type == DATE)*(Date *) data = expr.value.dateValue;
            else if (expr.type == STRING) {
                Date date{};
                TRY(Utils::transferStringToDate(expr.string.data(), date))
                *(Date *) data = date;
            } else return QL_UNSUPPORTED_ASSIGN_TYPE;
            break;
        }
        case VARCHAR: {
            if (expr.type == STRING) {
                Varchar varchar{};
                varchar.length = expr.string.length();
                if (varchar.length >= tableMeta.columns[columnId].stringMaxLength)return QL_VARCHAR_TOO_LONG;
                strcpy(varchar.spName, Utils::getStringPoolFileName(tableMeta.createName).c_str());
                TRY(spHandle.InsertString(expr.string.data(), varchar.length, varchar.offset))
                *(Varchar *) data = varchar;
            } else return QL_UNSUPPORTED_ASSIGN_TYPE;
            break;
        }
        default:
            return QL_UNSUPPORTED_ASSIGN_TYPE;
    }
    return OK_RC;
}