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
#include "../PS/PS_Expr.h"
#include "../utils/PrintError.h"

SM_Table::SM_Table(TableId _tableId) : tableId(_tableId), tableMeta(SM_Manager::Instance().GetTableMeta(tableId)) {
    init();
}

SM_Table::SM_Table(TableMeta &_tableMeta) : tableId(-1), tableMeta(_tableMeta) {
    init();
}

void SM_Table::init() {
    //printf("create %s\n", tableMeta.name);
    recordSize = 0;
    for (int i = 0; i < tableMeta.columnNum; i++) {
        columnOffset[i] = recordSize;
        recordSize += tableMeta.columns[i].attrLength + 1;//多一位来存储是否是NULL，放在数据的开头
        columnShowLength[i] = std::max(tableMeta.columns[i].stringMaxLength,
                                       std::max(tableMeta.columns[i].attrLength, COLUMN_SHOW_LENGTH));
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
            TRY(setColumnData(record + columnOffset[i], i, (*constValueList)[i]));
        }
    } else {
        //检测给定参数个数是否与行数相同
        if (columnIdList->size() != constValueList->size())return QL_COLUMNS_VALUES_DONT_MATCH;
        bool hasValue[tableMeta.columnNum];
        for (int i = 0; i < tableMeta.columnNum; i++)hasValue[i] = false;
        for (int i = 0; i < columnIdList->size(); i++) {
            hasValue[(*columnIdList)[i]] = true;
            TRY(setColumnData(record + columnOffset[i], (*columnIdList)[i], (*constValueList)[i]));
        }
        for (int i = 0; i < tableMeta.columnNum; i++)
            if (!hasValue[i]) {
                TRY(setColumnNull(record + columnOffset[i], i));
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
        TRY(composeIndexKeyByRecord(record, indexDesc, key))
        if (getIndexKeyCount(key, indexNo))return QL_PRIMARY_KEY_DUPLICATE;
    }
    //插入之前判断外键连接的主键的值是否存在，不存在则不能插入
    for (auto &foreignKey:tableMeta.foreignKeys)
        if (foreignKey.keyNum) {
            IndexDesc &index = tableMeta.indexes[foreignKey.indexIndex];
            char key[getIndexKeyLength(index)];
            composeIndexKeyByRecord(record, index, key);
            SM_Table table(foreignKey.primaryTable);
            if (table.getPrimaryKeyCount(key) == 0) {
                return QL_NO_INDICATE_PRIMARY_KEY_EXIST;
            }
        }
    TRY(rmFileHandle.InsertRec(record, rmRid))
    //插入索引
    for (int i = 0; i < MAX_INDEX_NUM; i++) {
        IndexDesc &index = tableMeta.indexes[i];
        if (index.keyNum) {
            char keyData[getIndexKeyLength(index)];
            TRY(composeIndexKeyByRecord(record, index, keyData));
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
    int lineLength = COLUMN_SHOW_LENGTH;
    for (int i = 0; i < tableMeta.columnNum; i++)lineLength += columnShowLength[i] + 1;
    std::string splitLine(lineLength, '-');
    std::cout << splitLine << std::endl;
    std::string headerLine;
    headerLine.append("id");
    headerLine.append(COLUMN_SHOW_LENGTH - 2, ' ');
    for (int i = 0; i < tableMeta.columnNum; i++) {
        std::string columnName = std::string(tableMeta.columns[i].name);
        headerLine.append(columnName);
        if (columnName.length() < columnShowLength[i])
            headerLine.append(columnShowLength[i] - columnName.length(), ' ');
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
        if (column.length() < columnShowLength[i])column.append(columnShowLength[i] - column.length(), ' ');
        line.append(column);
    }
    return line;
}

RC SM_Table::setColumnData(char *columnData, ColumnId columnId, AttrValue attrValue, bool alreadyComplete) {
    //命令里有传该参数的值，但也有可能是null
    if (attrValue.isNull) {
        TRY(setColumnNull(columnData, columnId))
        return OK_RC;
    }
    //先设为不是null
    columnData[0] = 0;
    char *data = columnData + 1;
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

RC SM_Table::setColumnNull(char *columnData, ColumnId columnId) {
    //命令里没传，所以认为是null，但要先看是不是有defaultValue
    if (tableMeta.columns[columnId].hasDefaultValue) {
        TRY(setColumnData(columnData, columnId, tableMeta.columns[columnId].defaultValue, true))
        return OK_RC;
    }
    if (tableMeta.columns[columnId].allowNull) {
        columnData[0] = 1;
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
            column = Utils::transferDateToString(date);
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
    //printf("delete %s\n", tableMeta.name);

    RM_Manager::Instance().CloseFile(rmFileHandle);
    SP_Manager::CloseStringPool(spHandle);
}

RC SM_Table::createIndex(int indexNo, IndexDesc indexDesc, bool allowDuplicate) {
    clock_t start_time = clock();
    int totalCount = 0;
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
        totalCount++;
        RM_RID rmRid;
        char *record;
        TRY(rmRecord.GetData(record))
        composeIndexKeyByRecord(record, indexDesc, key);
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
            composeIndexKeyByRecord(record, indexDesc, key);
            TRY(rmRecord.GetRid(rmRid))
            if (getIndexKeyCount(key, indexNo) > 1) {
                TRY(rmFileScan.CloseScan())
                TRY(IX_Manager::Instance().DestroyIndex(tableMeta.createName, indexNo))
                return SM_INDEX_NOT_ALLOW_DUPLICATE;
            }
        }
        TRY(rmFileScan.CloseScan())
    }
    auto cost_time = clock() - start_time;
    printf("建索引: 共计为%d个条目建立了索引，花费%.3f秒\n", totalCount, (float) cost_time / CLOCKS_PER_SEC);
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

RC SM_Table::composeIndexKeyByRecord(char *record, IndexDesc indexDesc, char *key) {
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

int SM_Table::getIndexKeyCount(char *key, int indexNo) {
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
    clock_t start_time = clock();
    RM_Record rmRecord;
    int deleteCount = 0;
    auto ridList = filter(conditionList);
    for (auto &rid:ridList) {
        rmFileHandle.GetRec(rid, rmRecord);
        deleteCount++;
        TRY(deleteRecord(rmRecord.getData(), rid))
    }
    auto cost_time = clock() - start_time;
    printf("删除: 成功删除%d条，花费%.3f秒\n", deleteCount, (float) cost_time / CLOCKS_PER_SEC);
    return OK_RC;
}

RC SM_Table::deleteRecord(char *record, const RM_RID &rmRid, bool influencePrimaryKey) {
    //删除之前判断主键的值是否被外联，外联的话不能删除，没有主键不用判断
    if (tableMeta.primaryKey.keyNum && influencePrimaryKey) {
        int indexNo = tableMeta.primaryKey.indexIndex;
        IndexDesc &indexDesc = tableMeta.indexes[indexNo];
        int indexKeyLength = getIndexKeyLength(indexDesc);
        char key[indexKeyLength];
        TRY(composeIndexKeyByRecord(record, indexDesc, key))
        for (auto &reference:tableMeta.primaryKey.references)
            if (reference.keyNum) {
                SM_Table table(reference.foreignTable);
                if (table.getForeignKeyCount(key, reference.indexIndex) > 0) {
                    return QL_INDICATE_PRIMARY_KEY_HAS_REFERENCE_FOREIGN_KEY;
                }
            }
    }
    TRY(rmFileHandle.DeleteRec(rmRid))
    //删除索引
    for (int i = 0; i < MAX_INDEX_NUM; i++) {
        IndexDesc &index = tableMeta.indexes[i];
        if (index.keyNum) {
            char keyData[getIndexKeyLength(index)];
            TRY(composeIndexKeyByRecord(record, index, keyData));
            IX_IndexHandle ixIndexHandle;
            TRY(IX_Manager::Instance().OpenIndex(tableMeta.createName, i, ixIndexHandle))
            TRY(ixIndexHandle.DeleteEntry(keyData, rmRid))
            TRY(IX_Manager::Instance().CloseIndex(ixIndexHandle));
        }
    }
    return OK_RC;
}

int SM_Table::count() {
    return rmFileHandle.GetRecordCount();
}

RC SM_Table::updateWhereConditionSatisfied(std::vector<std::pair<std::string, PS_Expr> > *assignExprList,
                                           std::vector<PS_Expr> *conditionList) {
    clock_t start_time = clock();
    bool influencePrimaryKey = false;
    std::vector<ColumnId> updateColumnIdList;
    for (auto &assignExpr: *assignExprList) {
        ColumnId columnId = SM_Manager::Instance().GetColumnIdFromName(tableId, assignExpr.first.data());
        if (columnId < 0) return SM_COLUMN_NOT_EXIST;
        for (int i = 0; i < tableMeta.primaryKey.keyNum; i++)
            if (tableMeta.primaryKey.columnId[i] == columnId)influencePrimaryKey = true;
        updateColumnIdList.push_back(columnId);
    }
    int updateCount = 0, updateSuccessCount = 0;
    RM_Record rmRecord;
    char newRecord[recordSize];
    auto ridList = filter(conditionList);
    for (auto &rid:ridList) {
        rmFileHandle.GetRec(rid, rmRecord);
        updateCount++;
        //拷贝一个新的出来，因为不能直接修改原来的值
        memcpy(newRecord, rmRecord.getData(), recordSize);
        //这里一定要先尝试赋值，因为先删再赋值有可能出错这样就凭白少了一条记录，如果更新不成功跳过这一条的
        bool updateSuccess = true;
        for (int i = 0; i < updateColumnIdList.size(); i++) {
            if (setColumnDataByExpr(newRecord + columnOffset[updateColumnIdList[i]], updateColumnIdList[i],
                                    (*assignExprList)[i].second) !=
                OK_RC) {
                updateSuccess = false;
                break;
            }
        }
        if (updateSuccess) {
            updateSuccessCount++;
            //这些基本不会出错
            TRY(deleteRecord(rmRecord.getData(), rid, influencePrimaryKey))
            TRY(insertRecord(newRecord, influencePrimaryKey));
        }
    }
    auto cost_time = clock() - start_time;
    printf("更新: 需要更新%d条，成功更新%d条，花费%.3f秒\n", updateCount, updateSuccessCount, (float) cost_time / CLOCKS_PER_SEC);
    return OK_RC;
}


RC SM_Table::setColumnDataByExpr(char *columnData, ColumnId columnId, PS_Expr &expr, bool alreadyComputed) {
    if (!alreadyComputed)TRY(expr.eval(*this, columnData - columnOffset[columnId]))
    //命令里有传该参数的值，但也有可能是null
    if (expr.value.isNull) {
        TRY(setColumnNull(columnData, columnId))
        return OK_RC;
    }
    //先设为不是null
    columnData[0] = 0;
    char *data = columnData + 1;
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

std::vector<RM_RID> SM_Table::filter(std::vector<PS_Expr> *conditionList) {
    clock_t start_time = clock();
    auto myCondition = new std::vector<PS_Expr>;
    for (const auto &expr : *conditionList) {
        //如果左边是该表里的列，右边是常数
        if (expr.left && expr.left->isColumn &&
            (expr.left->tableName.empty() || expr.left->tableName == tableMeta.name) &&
            tableMeta.getColumnIdByName(expr.left->columnName.data()) >= 0 &&
            expr.right && expr.right->type != UNKNOWN) {
            myCondition->push_back(expr);
        }
        //如果列在右边，需要转换
        if (expr.right && expr.right->isColumn &&
            (expr.right->tableName.empty() || expr.right->tableName == tableMeta.name) &&
            tableMeta.getColumnIdByName(expr.right->columnName.data()) >= 0 &&
            expr.left && expr.left->type != UNKNOWN) {
            PS_Expr reverseExpr = expr;
            std::swap(reverseExpr.left, reverseExpr.right);
            switch (expr.op) {
                case EQ_OP:
                case NE_OP: {
                    break;
                }
                case LT_OP: {
                    reverseExpr.op = GT_OP;
                    break;
                }
                case GT_OP: {
                    reverseExpr.op = LT_OP;
                    break;
                }
                case LE_OP: {
                    reverseExpr.op = GE_OP;
                    break;
                }
                case GE_OP: {
                    reverseExpr.op = LE_OP;
                    break;
                }
                default:
                    throw "it should not be here";
            }
            myCondition->push_back(reverseExpr);
        }
    }
    //尝试用每个索引来检索，看哪个效果更好
    int optimizeIndex = -1;
    //如果没有和自己有关的，那么就不用了
    if (!myCondition->empty()) {
        int maxSolvedColumn = 0;
        for (int i = 0; i < MAX_INDEX_NUM; i++) {
            IndexDesc &index = tableMeta.indexes[i];
            if (index.keyNum) {
                char key[getIndexKeyLength(index)];
                if (composeIndexKeyByExprList(myCondition, index, key) != NO_OP && index.keyNum > maxSolvedColumn) {
                    optimizeIndex = i;
                }
            }
        }
    }

    std::vector<RM_RID> ans;
    //如果没找到索引，则遍历全局找到满足条件的
    if (optimizeIndex == -1) {
        RM_FileScan rmFileScan;
        DO(rmFileScan.OpenScan(rmFileHandle))
        RM_Record rmRecord;
        while (rmFileScan.GetNextRec(rmRecord) == OK_RC) {
            RM_RID rmRid;
            rmRecord.GetRid(rmRid);
            bool conditionSatisfied = true;
            for (auto &condition:*myCondition) {
                DO(condition.eval(*this, rmRecord.getData()))
                conditionSatisfied &= condition.value.boolValue;
                if (!conditionSatisfied)break;
            }
            if (conditionSatisfied)ans.emplace_back(rmRid);
        }
        DO(rmFileScan.CloseScan())
    } else {
        //否则用索引来获取满足部分条件的列表，再重新filter满足所有条件的列表
        IndexDesc &index = tableMeta.indexes[optimizeIndex];
        IX_IndexHandle ixIndexHandle;
        DO(IX_Manager::Instance().OpenIndex(tableMeta.createName, optimizeIndex, ixIndexHandle))
        char key[getIndexKeyLength(index)];
        Operator op = composeIndexKeyByExprList(myCondition, index, key);
        IX_IndexScan ixIndexScan;
        DO(ixIndexScan.OpenScan(ixIndexHandle, op, key))
        RM_RID rmRid;
        while (ixIndexScan.GetNextEntry(rmRid) == OK_RC) {
            RM_Record rmRecord;
            rmFileHandle.GetRec(rmRid, rmRecord);
            bool conditionSatisfied = true;
            for (auto &condition:*myCondition) {
                DO(condition.eval(*this, rmRecord.getData()))
                conditionSatisfied &= condition.value.boolValue;
                if (!conditionSatisfied)break;
            }
            if (conditionSatisfied)ans.emplace_back(rmRid);
        }
        DO(ixIndexScan.CloseScan())
        DO(IX_Manager::Instance().CloseIndex(ixIndexHandle))
    }
    auto cost_time = clock() - start_time;
    //printf("过滤: 获得%zu条，花费%.3f秒\n", ans.size(), (float) cost_time / CLOCKS_PER_SEC);
    delete myCondition;
    return ans;
}

RC SM_Table::getRecordFromRID(RM_RID &rmRid, RM_Record &rmRecord) {
    TRY(rmFileHandle.GetRec(rmRid, rmRecord))
    return OK_RC;
}

RC SM_Table::clear() {
    std::string recordFileName = Utils::getRecordFileName(tableMeta.name);
    TRY(RM_Manager::Instance().DestroyFile(recordFileName.c_str()))
    std::string stringPoolFileName = Utils::getStringPoolFileName(tableMeta.name);
    TRY(SP_Manager::DestroyStringPool(stringPoolFileName.c_str()))
    //TODO 删除索引等
}

//必须保证该table是刚一条一条插入进来的，该操作结果才正确
RC SM_Table::orderBy(std::vector<const char *> *orderByColumn, bool increasingOrder, int limitOffset, int limitLength) {
    clock_t start_time = clock();
    int totalCount = 0;
    int orderCount = 0;
    //先构造出索引的meta，然后把index设为-1
    IndexDesc indexDesc;
    indexDesc.keyNum = orderByColumn->size();
    for (int i = 0; i < indexDesc.keyNum; i++) {
        ColumnId columnId = tableMeta.getColumnIdByName((*orderByColumn)[i]);
        if (columnId < 0)return SM_COLUMN_NOT_EXIST;
        indexDesc.columnId[i] = columnId;
    }
    TRY(createIndex(-1, indexDesc, true))
    IX_IndexHandle ixIndexHandle;
    TRY(IX_Manager::Instance().OpenIndex(tableMeta.createName, -1, ixIndexHandle))
    IX_IndexScan ixIndexScan;
    //打开一个无条件的遍历
    TRY(ixIndexScan.OpenScan(ixIndexHandle, NO_OP, nullptr))
    //先把顺序存下来
    std::vector<RM_RID> ridList;
    RM_RID rid;
    while (ixIndexScan.GetNextEntry(rid) == OK_RC)ridList.push_back(rid);
    if (!increasingOrder)std::reverse(ridList.begin(), ridList.end());
    //关掉扫描，关掉索引，删掉索引
    TRY(ixIndexScan.CloseScan())
    TRY(IX_Manager::Instance().CloseIndex(ixIndexHandle))
    TRY(IX_Manager::Instance().DestroyIndex(tableMeta.createName, -1))
    //把这期间的都再次插入
    int limitEnd;
    if (limitLength == -1)limitEnd = ridList.size();
    else limitEnd = std::min(int(ridList.size()), limitOffset + limitLength);
    for (int i = limitOffset; i < limitEnd; i++) {
        orderCount++;
        RM_Record rmRecord;
        TRY(rmFileHandle.GetRec(ridList[i], rmRecord))
        RM_RID rmRid;
        TRY(rmFileHandle.InsertRec(rmRecord.getData(), rmRid))
        //printf("insert rid is (%d %d)\n", rmRid.getPageNum(), rmRid.getSlotNum());
    }
    //然后把之前的全部删掉
    for (auto &rmRid:ridList) {
        totalCount++;
        //printf("delete rid is (%d %d)\n", rmRid.getPageNum(), rmRid.getSlotNum());
        TRY(rmFileHandle.DeleteRec(rmRid))
    }
    auto cost_time = clock() - start_time;
    printf("排序: 共计%d条，成功按序选取%d条，花费%.3f秒\n", totalCount, orderCount, (float) cost_time / CLOCKS_PER_SEC);
    return OK_RC;
}

std::vector<PS_Expr> *SM_Table::extractValueInRecords() {
    auto ret = new std::vector<PS_Expr>();
    RM_FileScan rmFileScan;
    rmFileScan.OpenScan(rmFileHandle);
    RM_Record rmRecord;
    auto expr = new PS_Expr(nullptr, tableMeta.columns[0].name);
    while (rmFileScan.GetNextRec(rmRecord) == OK_RC) {
        expr->eval(*this, rmRecord.getData());
        ret->push_back(*expr);
    }
    rmFileScan.CloseScan();
    return ret;
}

Operator SM_Table::composeIndexKeyByExprList(std::vector<PS_Expr> *exprList, IndexDesc indexDesc, char *key) {
    if (indexDesc.keyNum == 1) {
        ColumnId columnId = indexDesc.columnId[0];
        return findAndCopy(columnId, exprList, key);
    } else {
        //联合索引的key组成形式为attrType1 + attrLength1 + attrValue1 + attrType2 + attrLength2 + attrValue2.....
        int length = 0;
        for (int i = 0; i < indexDesc.keyNum; i++) {
            ColumnId columnId = indexDesc.columnId[i];
            *(AttrType *) (key + length) = tableMeta.columns[columnId].attrType;
            *(int *) (key + length + ATTR_TYPE_LENGTH) = tableMeta.columns[columnId].attrLength;
            if (findAndCopy(columnId, exprList, key + length + ATTR_TYPE_LENGTH + 4) != EQ_OP) {
                return NO_OP;
            }
            length += tableMeta.columns[columnId].attrLength + ATTR_TYPE_LENGTH + 4;
        }
        return EQ_OP;
    }
}

Operator SM_Table::findAndCopy(ColumnId columnId, std::vector<PS_Expr> *exprList, char *key) {
    for (auto &expr: *exprList)
        if (tableMeta.getColumnIdByName(expr.left->columnName.data()) == columnId) {
            switch (tableMeta.columns[columnId].attrType) {
                case INT: {
                    memcpy(key, &expr.right->value.intValue, tableMeta.columns[columnId].attrLength);
                    break;
                }
                case FLOAT: {
                    memcpy(key, &expr.right->value.floatValue, tableMeta.columns[columnId].attrLength);
                    break;
                }
                case STRING: {
                    memcpy(key, expr.right->string.data(), tableMeta.columns[columnId].attrLength);
                    break;
                }
                case DATE: {
                    Date date;
                    date.year = date.month = date.day = -1;
                    Utils::transferStringToDate(expr.right->string.data(), date);
                    memcpy(key, &date, tableMeta.columns[columnId].attrLength);
                    break;
                }
                case VARCHAR: {
                    //只存一次，防止多存
                    if (expr.right->value.varcharValue.length == 0) {
                        Varchar varchar{};
                        varchar.length = expr.right->string.size();
                        strcpy(varchar.spName, Utils::getStringPoolFileName(tableMeta.createName).c_str());
                        spHandle.InsertString(expr.right->string.data(), varchar.length, varchar.offset);
                        expr.right->value.varcharValue = varchar;
                    }
                    memcpy(key, &expr.right->value.varcharValue, tableMeta.columns[columnId].attrLength);
                    break;
                }
                default:
                    throw "unsupported index type";
            }
            return expr.op;
        }
    return NO_OP;
}

int SM_Table::getPrimaryKeyCount(char *key) {
    return getIndexKeyCount(key, tableMeta.primaryKey.indexIndex);
}

int SM_Table::getForeignKeyCount(char *key, int foreignKeyIndexIndex) {
    return getIndexKeyCount(key, foreignKeyIndexIndex);
}

bool SM_Table::validForeignKey(IndexDesc indexDesc, TableId primaryTableId) {
    char key[getIndexKeyLength(indexDesc)];
    //打开一个主键表用来判断
    SM_Table table(primaryTableId);
    //遍历所有记录
    RM_FileScan rmFileScan;
    rmFileScan.OpenScan(rmFileHandle);
    RM_Record rmRecord;
    while (rmFileScan.GetNextRec(rmRecord) == OK_RC) {
        composeIndexKeyByRecord(rmRecord.getData(), indexDesc, key);
        if (table.getPrimaryKeyCount(key) == 0) {
            return false;
        }
    }
    rmFileScan.CloseScan();
    return true;
}
