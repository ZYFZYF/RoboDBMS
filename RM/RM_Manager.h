//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBODBMS_RM_MANAGER_H
#define ROBODBMS_RM_MANAGER_H


#include "../PF/PF_Manager.h"
#include "RM_FileHandle.h"

class RM_Manager {
public:
    RM_Manager(PF_Manager &pfm);

    ~RM_Manager();

    RC CreateFile(const char *fileName, int recordSize);

    RC DestroyFile(const char *fileName);

    RC OpenFile(const char *fileName, RM_FileHandle &fileHandle);

    RC CloseFile(RM_FileHandle &fileHandle);

private:
    PF_Manager pfm;
};


#endif //ROBODBMS_RM_MANAGER_H
