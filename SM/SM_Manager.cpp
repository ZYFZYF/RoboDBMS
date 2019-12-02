//
// Created by 赵鋆峰 on 2019/12/2.
//

#include "SM_Manager.h"

SM_Manager::SM_Manager(IX_Manager &ixm_, RM_Manager &rmm_) {

}

SM_Manager::~SM_Manager() {

}

RC SM_Manager::OpenDb(const char *dbName) {
    return PF_NOBUF;
}

RC SM_Manager::CloseDb() {
    return PF_NOBUF;
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
    return PF_NOBUF;
}

RC SM_Manager::DropDb(const char *dbName) {
    return PF_NOBUF;
}

RC SM_Manager::UseDb(const char *dbName) {
    return PF_NOBUF;
}
