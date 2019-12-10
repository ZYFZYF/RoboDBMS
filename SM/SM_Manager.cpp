//
// Created by 赵鋆峰 on 2019/12/2.
//

#include "SM_Manager.h"
#include "../utils/Utils.h"

SM_Manager::SM_Manager() : pfManager(PF_Manager::Instance()), rmManager(RM_Manager::Instance()),
                           ixManager(IX_Manager::Instance()), isUsingDb(false) {

    //Meta文件中存储了所有的数据库管理系统的信息
    dbmsMetaFile = Utils::getAbsolutePath("DBMS.Meta");
    //如果没有该文件，那么要初始化一个带meta的meta文件，否则从该文件中读取meta
    if (access(dbmsMetaFile.c_str(), F_OK) < 0) {
        pfManager.CreateFileWithMeta(dbmsMetaFile.c_str(), &dbmsMeta, sizeof(DbmsMeta));
    } else {
        pfManager.GetMeta(dbmsMetaFile.c_str(), &dbmsMeta, sizeof(DbmsMeta));
    }
}

SM_Manager::~SM_Manager() {

}

RC SM_Manager::OpenDb(const char *dbName) {
    if (chdir(dbName) < 0) {
        return SM_DB_NOT_EXIST;
    }
    //打开的时候读，修改的话立马写，不用等到关的时候再写
    ReadDbMeta();
    return OK_RC;
}

RC SM_Manager::CloseDb() {
    //以防万一关闭的时候还是再写一次
    WriteDbMeta();
    chdir("..");
    return OK_RC;
}

RC SM_Manager::DropTable(const char *relName) {
    return PF_NOBUF;
}

RC SM_Manager::CreateIndex(const char *relName, const char *attrName) {
    return PF_NOBUF;
}

RC SM_Manager::DropIndex(const char *relName, const char *attrName) {
    return PF_NOBUF;
}

RC SM_Manager::Load(const char *relName, const char *fileName) {
    return PF_NOBUF;
}

RC SM_Manager::Help() {
    return PF_NOBUF;
}

RC SM_Manager::Help(const char *relName) {
    return PF_NOBUF;
}

RC SM_Manager::Print(const char *relName) {
    return PF_NOBUF;
}

RC SM_Manager::Set(const char *paramName, const char *value) {
    return PF_NOBUF;
}

RC SM_Manager::CreateDb(const char *dbName) {
    for (auto &db : dbmsMeta.databaseName)
        if (strcmp(db, dbName) == 0) {
            return SM_DB_ALREADY_IN;
        }
    for (auto &i : dbmsMeta.databaseName) {
        if (strlen(i) == 0) {
            strcpy(i, dbName);
            if (mkdir(dbName, S_IRWXU) < 0) {
                return SM_UNIX;
            }
            TRY(pfManager.UpdateMeta(dbmsMetaFile.c_str(), &dbmsMeta, sizeof(DbmsMeta)));
            return OK_RC;
        }
    }
    return SM_DB_IS_FULL;
}

RC SM_Manager::DropDb(const char *dbName) {
    for (auto &db : dbmsMeta.databaseName)
        if (strcmp(db, dbName) == 0) {
            memset(db, 0, sizeof(db));
            if (rmdir(dbName) < 0) {
                return SM_UNIX;
            }
            TRY(pfManager.UpdateMeta(dbmsMetaFile.c_str(), &dbmsMeta, sizeof(DbmsMeta)));
            return OK_RC;
        }
    return SM_DB_NOT_EXIST;
}

RC SM_Manager::UseDb(const char *dbName) {
    if (isUsingDb) {
        TRY(CloseDb());
    }
    TRY(OpenDb(dbName));
    isUsingDb = true;
    return OK_RC;
}

RC SM_Manager::AddColumn(const char *dbName, ColumnDesc columnDesc) {
    return PF_NOBUF;
}

RC SM_Manager::DropColumn(const char *dbName, const char *columnName) {
    return PF_NOBUF;
}

SM_Manager &SM_Manager::Instance() {
    static SM_Manager instance;
    return instance;
}

RC SM_Manager::ShowDatabases() {
    int cnt = 0;
    for (auto &db : dbmsMeta.databaseName)
        if (strlen(db) > 0) {
            cnt++;
            printf("%20s", db);
            if (++cnt % 5 == 0) {
                printf("\n");
            }
        }
    printf("\n");
    return OK_RC;
}

RC SM_Manager::DescDatabase(const char &dbName) {
    return PF_EOF;
}

RC SM_Manager::ShowTables() {
    if (!isUsingDb) {
        return SM_NOT_IN_DATABASE;
    }
    int cnt = 0;
    for (auto &tb : dbMeta.tableNames)
        if (strlen(tb) > 0) {
            cnt++;
            printf("%20s", tb);
            if (++cnt % 5 == 0) {
                printf("\n");
            }
        }
    printf("\n");
    return OK_RC;
}

RC SM_Manager::DescTable(const char *tbName) {
    return PF_EOF;
}

RC SM_Manager::CreateTable(const char *tbName, std::vector<ColumnDesc> *columnList) {
    if (!isUsingDb) {
        return SM_NOT_IN_DATABASE;
    }
    for (int i = 0; i < MAX_TABLE_NUM; i++)
        if (strlen(dbMeta.tableNames[i]) == 0) {
            strcpy(dbMeta.tableNames[i], tbName);
            TableMeta tableMeta{};
            strcpy(tableMeta.createName, tbName);
            for (int j = 0; j < columnList->size(); j++) {
                tableMeta.columns[j] = (*columnList)[j];
            }
            dbMeta.tableMetas[i] = tableMeta;
            WriteDbMeta();
            return OK_RC;
        }
    return SM_TABLE_IS_FULL;
}

RC SM_Manager::ReadDbMeta() {
    //不存在的话要新建
    if (access("DB.Meta", F_OK) < 0) {
        memset(&dbMeta, 0, sizeof(dbMeta));
        WriteDbMeta();
    }
    FILE *file = fopen("DB.Meta", "r");
    if (fread(&dbMeta, sizeof(dbMeta), 1, file) != 1) {
        return SM_UNIX;
    }
    fclose(file);
    return OK_RC;
}

RC SM_Manager::WriteDbMeta() {
    FILE *file = fopen("DB.Meta", "w");
    if (fwrite(&dbMeta, sizeof(dbMeta), 1, file) != 1) {
        return SM_UNIX;
    }
    fclose(file);
    return OK_RC;
}


