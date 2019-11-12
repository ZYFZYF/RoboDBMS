//
// Created by 赵鋆峰 on 2019/11/12.
//

#ifndef ROBODBMS_SP_MANAGER_H
#define ROBODBMS_SP_MANAGER_H


#include "../def.h"
#include "SP_Handle.h"

class SP_Manager {
    RC CreateStringPool(const char *fileName);

    RC DestroyStringPool(const char *fileName);

    RC OpenStringPool(const char *fileName, SP_Handle &spHandle);

    RC CloseStringPool(SP_Handle &spHandle);

};


#endif //ROBODBMS_SP_MANAGER_H
