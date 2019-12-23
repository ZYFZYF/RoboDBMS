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

    ~SM_Manager();                             // Destructor

    RC OpenDb(const char *dbName);           // Open the database

    RC CloseDb();                             // close the database

    RC CreateTable(const char *tbName, std::vector<ColumnDesc> *columnList);

    RC DropTable(const char *tbName);          // destroy a relation

    RC CreateIndex(const char *relName,           // create an index for
                   const char *attrName);         //   relName.attrName
    RC DropIndex(const char *relName,           // destroy index on
                 const char *attrName);         //   relName.attrName

    RC Load(const char *relName,           // load relName from
            const char *fileName);         //   fileName

    RC Help();                             // Print relations in db
    RC Help(const char *relName);          // print schema of relName

    RC Print(const char *relName);          // print relName contents

    RC Set(const char *paramName,         // set parameter to
           const char *value);            //   value

    //课程中额外要求的操作
    RC CreateDb(const char *dbName);

    RC DropDb(const char *dbName);

    RC UseDb(const char *dbName);

    RC AddColumn(const char *dbName, ColumnDesc columnDesc);

    RC DropColumn(const char *dbName, const char *columnName);

    //SHOW 相关操作

    RC ShowDatabases();

    RC DescDatabase(const char &dbName);

    RC ShowTables();

    RC ShowTable(const char *tbName);

    RC DescTable(const char *tbName);

    RC AddPrimaryKey(const char *table, std::vector<const char *> *columns);

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

    TableMeta &GetTableMeta(TableId tableId);

private:
    SM_Manager();

    DbMeta dbMeta, dbMetaBackup;

    RC ReadDbMeta();//读取当前数据库的Meta信息到dbMeta中

    RC WriteDbMeta();//将dbMeta写回到文件中

    RC RecoverDbMeta();//操作中出现错误，将meta回滚



    PF_Manager pfManager;
    RM_Manager rmManager;
    IX_Manager ixManager;
    bool isUsingDb;

    PF_FileHandle dbmsMetaFileHandle;
    DbmsMeta dbmsMeta;
    std::string dbmsMetaFile;


};


#endif //ROBODBMS_SM_MANAGER_H
