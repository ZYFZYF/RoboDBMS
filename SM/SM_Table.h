//
// Created by 赵鋆峰 on 2019/12/21.
//

#ifndef ROBODBMS_SM_TABLE_H
#define ROBODBMS_SM_TABLE_H


#include <vector>
#include "SM_Constant.h"
#include "../Attr.h"
#include "../RM/RM_FileHandle.h"
#include "../PS/PS_Expr.h"

#define COLUMN_SHOW_LENGTH  30 //一列显示的时候占的宽度

class SM_Table {
    friend class PS_Expr;

public:
    explicit SM_Table(const TableMeta &tableMeta);

    char *getColumnData(char *record, ColumnId columnId);//获取某一列的数据，如果没有返回NULL

    RC
    setRecordData(char *record, std::vector<ColumnId> *columnIdList, std::vector<AttrValue> *constValueList);//给一列设置数据

    RC insertRecord(char *record);//插入一行记录

    RC deleteRecord(char *record, const RM_RID &rmRid);//删除一行记录

    RC deleteWhereConditionSatisfied(std::vector<PS_Expr> *conditionList);

    RC updateWhereConditionSatisfied(std::vector<std::pair<std::string, PS_Expr> > *assignExprList,
                                     std::vector<PS_Expr> *conditionList);

    void showRecords(int num);//显示头多少条记录，如果是-1代表显示所有

    std::string formatColumnToString(ColumnId columnId, char *data);

    std::string formatAttrValueToString(ColumnId columnId, AttrValue);

    RC completeAttrValueByColumnId(ColumnId columnId, AttrValue &attrValue);

    RC createIndex(int indexNo, IndexDesc indexDesc, bool allowDuplicate = true);

    int getIndexKeyLength(IndexDesc indexDesc);

    RC composeIndexKey(char *record, IndexDesc indexDesc, char *key);

    //获取一个key在index中出现了多少次
    int getIndexKeyDuplicateNum(char *key, int indexNo, IndexDesc indexDesc);

    int count();

    ~SM_Table();

private:
    TableMeta tableMeta;
    int recordSize;
public:
    int getRecordSize() const;

private:
    RM_FileHandle rmFileHandle;
    SP_Handle spHandle;
    int columnOffset[MAX_COLUMN_NUM];

    std::string formatRecordToString(char *record);


    RC setColumnData(char *record, ColumnId columnId, AttrValue attrValue, bool alreadyComplete = false);

    RC setColumnNull(char *record, ColumnId columnId);
};

#endif //ROBODBMS_SM_TABLE_H
