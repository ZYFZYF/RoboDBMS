//
// Created by 赵鋆峰 on 2019/12/2.
//

#ifndef ROBODBMS_SM_MANAGER_H
#define ROBODBMS_SM_MANAGER_H


#include "../RM/RM_Manager.h"
#include "../IX/IX_Manager.h"
#include "SM_Constant.h"
#include<vector>


class SM_Manager {
public:
    static SM_Manager &Instance();

    ~SM_Manager();

    RC OpenDb(const char *dbName);

    RC CloseDb();

    RC CreateTable(const char *tbName, std::vector<ColumnDesc> *columnList);

    RC DropTable(const char *tbName);

    RC AddIndex(const char *tbName, const char *indexName, std::vector<const char *> *columns);

    RC DropIndex(const char *tbName, const char *indexName);

    RC Load(const char *relName,
            const char *fileName);

    RC Help();

    RC Help(const char *relName);

    RC Print(const char *relName);

    RC Set(const char *paramName,
           const char *value);

    //课程中额外要求的操作
    RC CreateDb(const char *dbName);

    RC DropDb(const char *dbName);

    RC UseDb(const char *dbName);

    RC AddColumn(const char *tbName, ColumnDesc columnDesc);

    RC DropColumn(const char *tbName, const char *columnName);

    RC UpdateColumn(const char *tbName, const char *columnName, const char *newColumnName);

    RC UpdateColumn(const char *tbName, const char *columnName, ColumnDesc columnDesc);

    //SHOW 相关操作

    RC ShowDatabases();

    RC DescDatabase(const char &dbName);

    RC ShowTables();

    RC ShowTable(const char *tbName);

    RC DescTable(const char *tbName);

    RC AddPrimaryKey(const char *tbName, std::vector<const char *> *columns);

    RC
    AddForeignKey(const char *name, const char *foreignTable, std::vector<const char *> *foreignColumns,
                  const char *primaryTable,
                  std::vector<const char *> *primaryColumns);

    RC DropPrimaryKey(const char *table);

    RC DropForeignKey(const char *foreignTable, const char *name);

    const char *GetTableNameFromTableId(TableId tableId);

    const char *GetColumnNameFromId(TableId tableId, ColumnId columnId);

    TableId GetTableIdFromName(const char *tableName);

    ColumnId GetColumnIdFromName(const char *tableName, const char *columnName);

    ColumnId GetColumnIdFromName(TableId tableId, const char *columnName);

    AttrType GetType(TableId tableId, ColumnId columnId);

    TableMeta &GetTableMeta(TableId tableId);

    TableMeta &GetTableMeta(const char *tbName);

    RC RenameTable(const char *oldTbName, const char *newTbName);

private:
    SM_Manager();

    DbMeta dbMeta;

    RC ReadDbMeta();//读取当前数据库的Meta信息到dbMeta中

    RC WriteDbMeta();//将dbMeta写回到文件中


    PF_Manager pfManager;
    RM_Manager rmManager;
    IX_Manager ixManager;
    bool isUsingDb;

    PF_FileHandle dbmsMetaFileHandle;
    DbmsMeta dbmsMeta;
    std::string dbmsMetaFile;


};


#endif //ROBODBMS_SM_MANAGER_H
