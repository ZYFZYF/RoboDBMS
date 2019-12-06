//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBODBMS_RM_MANAGER_H
#define ROBODBMS_RM_MANAGER_H


#include "../PF/PF_Manager.h"
#include "RM_FileHandle.h"

class RM_Manager {
public:
    static RM_Manager &Instance();

    ~RM_Manager();

    RC CreateFile(const char *fileName, int recordSize);

    RC DestroyFile(const char *fileName);

    RC OpenFile(const char *fileName, RM_FileHandle &rmFileHandle);

    RC CloseFile(RM_FileHandle &rmFileHandle);

private:
    RM_Manager();

    PF_Manager pfManager;
};


#endif //ROBODBMS_RM_MANAGER_H
