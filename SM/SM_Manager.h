//
// Created by 赵鋆峰 on 2019/12/2.
//

#ifndef ROBODBMS_SM_MANAGER_H
#define ROBODBMS_SM_MANAGER_H


#include "../RM/RM_Manager.h"
#include "../IX/IX_Manager.h"
#include "SM_Constant.h"


struct AttrInfo {
    int x;
};

class SM_Manager {
public:
    static SM_Manager &Instance();

    ~SM_Manager();                             // Destructor

    RC OpenDb(const char *dbName);           // Open the database

    RC CloseDb();                             // close the database

    RC CreateTable(const char *relName,           // create relation relName
                   int attrCount,          //   number of attributes
                   AttrInfo *attributes);       //   attribute data
    RC DropTable(const char *relName);          // destroy a relation

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

private:
    SM_Manager();

    IX_Manager ixManager;
    RM_Manager rmManager;
    bool isUsingDb;
};


#endif //ROBODBMS_SM_MANAGER_H
