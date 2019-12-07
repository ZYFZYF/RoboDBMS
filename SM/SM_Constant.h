//
// Created by 赵鋆峰 on 2019/12/2.
//

#ifndef ROBODBMS_SM_CONSTANT_H
#define ROBODBMS_SM_CONSTANT_H

#include <cstring>
#include <cassert>
#include "../Constant.h"

#define MAX_DATABASE_NUM 100//一个数据库管理系统中最多的数据库个数
#define MAX_TABLE_NUM 30 //一个数据库中最多的表的数目
#define MAX_COLUMN_NUM 30 //一个表中最多的列的个数
#define MAX_FOREIGN_KEY_NUM 30 //一个表中最多有多少组外键
#define MAX_INDEX_NUM 30 //一个表最多建多少个索引
#define INVALID_INDEX -1
#define TABLE_META_LENGTH sizeof(struct TableMeta)
#define DB_META_LENGTH sizeof(struct DbMeta)

typedef int ColumnId;

struct DbmsMeta {
    char databaseName[MAX_DATABASE_NUM][MAX_NAME_LENGTH]{};
};

struct DbMeta {
    char tableName[MAX_TABLE_NUM][MAX_NAME_LENGTH]{};
};

//描述唯一主键
struct PrimaryKeyDesc {
    char name[MAX_NAME_LENGTH];
    int keyNum;//因为联合主键的存在
    ColumnId columnId[MAX_COLUMN_NUM];
};


//描述一个列
struct ColumnDesc {
    char name[MAX_NAME_LENGTH];
    bool allowNull;
    bool hasDefaultValue;
    AttrType attrType;
    AttrValue defaultValue;
    int maxLength;//定长字符串或者小数类型时用
    int decimalLength;//只有在是小数类型时用
};

//描述一个外键约束，注意有可能一列当了多个外键，或者联合外键这种东西
struct ForeignKeyDesc {
    char name[MAX_NAME_LENGTH];
    int keyNum;
    ColumnId local[MAX_COLUMN_NUM];
    char foreignTable[MAX_NAME_LENGTH];
    ColumnId foreign[MAX_COLUMN_NUM];
};

//描述一个索引
struct IndexDesc {
    char name[MAX_NAME_LENGTH];
    ColumnId columnId[MAX_COLUMN_NUM];
};

//每个的源信息，存在文件头里
struct TableMeta {
    //建表的时候的名字，以后改名之后仍然用这个来建索引等等
    char createName[MAX_NAME_LENGTH];
    //给每个列分配一个独一无二的id，放在桶里
    int availableColumnIdNum;
    ColumnId columnIdBucket[MAX_COLUMN_NUM];
    //主键
    PrimaryKeyDesc primaryKey;
    //外键
    ForeignKeyDesc foreignKeys[MAX_FOREIGN_KEY_NUM];
    //索引
    IndexDesc indexes[MAX_INDEX_NUM];
    //列描述
    ColumnDesc columns[MAX_COLUMN_NUM];

    int getColumnId(const char *columnName) {
        for (int i = availableColumnIdNum; i < MAX_TABLE_NUM; i++)
            if (strcmp(columns[columnIdBucket[i]].name, columnName) == 0) {
                return i;
            }
        return INVALID_INDEX;
    }

    RC insertColumn(const char *columnName) {
        if (columnName == nullptr || strlen(columnName) > MAX_NAME_LENGTH)
            return SM_INVALID_NAME;
        int index = getColumnId(columnName);
        if (index != INVALID_INDEX) {
            return SM_COLUMN_ALREADY_IN;
        }
        if (availableColumnIdNum) {
            strcpy(columns[columnIdBucket[--availableColumnIdNum]].name, columnName);
            return OK_RC;
        } else {
            return SM_COLUMN_IS_FULL;
        }
    }

    RC deleteColumn(ColumnId columnId) {
        assert(availableColumnIdNum < MAX_TABLE_NUM);
        columnIdBucket[availableColumnIdNum++] = columnId;
        return OK_RC;
    }
};

#endif //ROBODBMS_SM_CONSTANT_H
