//
// Created by 赵鋆峰 on 2019/12/21.
//

#ifndef ROBODBMS_SM_TABLE_H
#define ROBODBMS_SM_TABLE_H


#include <vector>
#include "SM_Constant.h"
#include "../Attr.h"
#include "../RM/RM_FileHandle.h"

#define COLUMN_SHOW_LENGTH  30 //一列显示的时候占的宽度

class PS_Expr;

class SM_Table {
    friend class PS_Expr;

    friend class QL_MultiTable;

public:
    //通常情况下是从存在的表中新建
    explicit SM_Table(TableId _tableId);

    //有时候会从虚拟的tableMeta生成表，还可用于后面的Select实现
    explicit SM_Table(TableMeta &_tableMeta);


    char *getColumnData(char *record, ColumnId columnId);//获取某一列的数据，如果没有返回NULL

    RC
    setRecordData(char *record, std::vector<ColumnId> *columnIdList, std::vector<AttrValue> *constValueList);//给一列设置数据

    //是否影响主键
    RC insertRecord(char *record, bool influencePrimaryKey = true);//插入一行记录

    RC deleteRecord(char *record, const RM_RID &rmRid, bool influencePrimaryKey = true);//删除一行记录

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

    RC clear();

    RC getRecordFromRID(RM_RID &rmRid, RM_Record &rmRecord);

    //从一系列限制中返回一个RM_RID list
    std::vector<RM_RID> filter(std::vector<PS_Expr> *conditionList);

    int getRecordSize() const;

private:
    TableId tableId;
    TableMeta &tableMeta;
    int recordSize{};
    RM_FileHandle rmFileHandle;
    SP_Handle spHandle;
    int columnOffset[MAX_COLUMN_NUM]{};

    void init();

    std::string formatRecordToString(char *record);

    RC setColumnData(char *columnData, ColumnId columnId, AttrValue attrValue, bool alreadyComplete = false);

    RC setColumnNull(char *columnData, ColumnId columnId);

    RC setColumnDataByExpr(char *columnData, ColumnId columnId, PS_Expr &expr, bool alreadyComputed = false);
};

#endif //ROBODBMS_SM_TABLE_H
