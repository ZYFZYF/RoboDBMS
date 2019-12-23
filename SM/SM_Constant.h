//
// Created by 赵鋆峰 on 2019/12/2.
//

#ifndef ROBODBMS_SM_CONSTANT_H
#define ROBODBMS_SM_CONSTANT_H

#include <cstring>
#include <cassert>
#include "../Constant.h"
#include "../Attr.h"

#define MAX_DATABASE_NUM 100//一个数据库管理系统中最多的数据库个数
#define MAX_TABLE_NUM 30 //一个数据库中最多的表的数目
#define MAX_COLUMN_NUM 30 //一个表中最多的列的个数
#define MAX_FOREIGN_KEY_NUM 30 //一个表中最多有多少组外键
#define MAX_REFERENCE_NUM 30 //一个主键最多被多少个外键引用
#define MAX_INDEX_NUM 30 //一个表最多建多少个索引
#define INVALID_INDEX -1
#define TABLE_META_LENGTH sizeof(struct TableMeta)
#define DB_META_LENGTH sizeof(struct DbMeta)

typedef int ColumnId;
typedef int TableId;

struct DbmsMeta {
    char databaseName[MAX_DATABASE_NUM][MAX_NAME_LENGTH]{};
};


//描述一个列
struct ColumnDesc {
    char name[MAX_NAME_LENGTH];
    AttrType attrType;
    int attrLength;
    int stringMaxLength;
    int integerLength;
    int decimalLength;//只有在是小数类型时用
    bool allowNull;
    bool hasDefaultValue;
    AttrValue defaultValue;
    bool isPrimaryKey;
    bool hasForeignKey;
    char primaryKeyTable[MAX_NAME_LENGTH];
    char primaryKeyColumn[MAX_NAME_LENGTH];
};

//描述一个外键约束，注意有可能一列当了多个外键，或者联合外键这种东西
struct ForeignKeyDesc {
    char name[MAX_NAME_LENGTH];
    int keyNum;
    TableId foreignTable;
    ColumnId foreign[MAX_COLUMN_NUM];
    TableId primaryTable;
    ColumnId primary[MAX_COLUMN_NUM];
};

//描述唯一主键
struct PrimaryKeyDesc {
    char name[MAX_NAME_LENGTH];
    int keyNum;//因为联合主键的存在
    ColumnId columnId[MAX_COLUMN_NUM];
    //因为会有别的外键引用到这个主键，所以要记录下来
    int referenceNum;
    ForeignKeyDesc references[MAX_TABLE_NUM];
};

//描述一个索引
struct IndexDesc {
    char name[MAX_NAME_LENGTH];
    int keyNum;
    ColumnId columnId[MAX_COLUMN_NUM];
};

//每个的源信息，存在文件头里
struct TableMeta {
    //建表的时候的名字，以后改名之后仍然用这个来建索引等等
    char createName[MAX_NAME_LENGTH];
    //主键
    PrimaryKeyDesc primaryKey;
    //外键
    ForeignKeyDesc foreignKeys[MAX_FOREIGN_KEY_NUM];
    //索引
    IndexDesc indexes[MAX_INDEX_NUM];
    //列描述，列的存储必须是从下标0到columnNum-1
    int columnNum;
    ColumnDesc columns[MAX_COLUMN_NUM];
};

struct DbMeta {
    char tableNames[MAX_TABLE_NUM][MAX_NAME_LENGTH]{};
    TableMeta tableMetas[MAX_TABLE_NUM]{};
};

#endif //ROBODBMS_SM_CONSTANT_H
