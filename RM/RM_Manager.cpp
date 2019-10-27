//
// Created by 赵鋆峰 on 2019/10/27.
//

#include "RM_Manager.h"

RM_Manager::RM_Manager(PF_Manager &pfm) {
    this->pfm = pfm;
}

RM_Manager::~RM_Manager() {

}

RC RM_Manager::CreateFile(const char *fileName, int recordSize) {
    if (recordSize > PF_PAGE_SIZE) {
        return RM_TOOLARGERECORDSIZE;
    }
    return pfm.CreateFile(fileName);
}

RC RM_Manager::DestroyFile(const char *fileName) {
    return pfm.DestroyFile(fileName);
}

RC RM_Manager::OpenFile(const char *fileName, RM_FileHandle &fileHandle) {
    RC rc;
    if (rc = pfm.OpenFile(fileName,))

}

RC RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
    return PF_NOBUF;
}
