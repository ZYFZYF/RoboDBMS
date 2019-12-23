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

class SM_Table {
public:
    explicit SM_Table(const TableMeta &tableMeta);

    char *getColumnData(char *record, ColumnId columnId);//获取某一列的数据，如果没有返回NULL

    RC
    setRecordData(char *record, std::vector<ColumnId> *columnIdList, std::vector<AttrValue> *constValueList);//给一列设置数据

    RC insertRecord(const char *record);//插入一行记录

    void showRecords(int num);//显示头多少条记录，如果是-1代表显示所有

    std::string formatColumnToString(ColumnId columnId, char *data);

    std::string formatAttrValueToString(ColumnId columnId, AttrValue);

    RC completeAttrValueByColumnId(ColumnId columnId, AttrValue &attrValue);


private:
    TableMeta tableMeta;
    int recordSize;
    RM_FileHandle rmFileHandle;
    SP_Handle spHandle;
    int columnOffset[MAX_COLUMN_NUM];

    std::string formatRecordToString(char *record);


    RC setColumnData(char *record, ColumnId columnId, AttrValue attrValue);

    RC setColumnNull(char *record, ColumnId columnId);
};


#endif //ROBODBMS_SM_TABLE_H
