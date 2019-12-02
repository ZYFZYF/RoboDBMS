//
// Created by 赵鋆峰 on 2019/12/2.
//

#include "SM_Manager.h"

SM_Manager::SM_Manager(IX_Manager &ixm_, RM_Manager &rmm_) : ixManager(ixm_), rmManager(rmm_), isUsingDb(false) {

}

SM_Manager::~SM_Manager() {

}

RC SM_Manager::OpenDb(const char *dbName) {
    if (chdir(dbName) < 0) {
        return SM_DB_NOT_EXIST;
    }
    return OK_RC;
}

RC SM_Manager::CloseDb() {
    chdir("..");
}

RC SM_Manager::CreateTable(const char *relName, int attrCount, AttrInfo *attributes) {
    return PF_NOBUF;
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
    if (mkdir(dbName, S_IRWXU) < 0) {
        return SM_DB_ALREADY_IN;
    }
    return OK_RC;
}

RC SM_Manager::DropDb(const char *dbName) {
    if (rmdir(dbName) < 0) {
        return SM_DB_NOT_EXIST;
    }
    return OK_RC;
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
