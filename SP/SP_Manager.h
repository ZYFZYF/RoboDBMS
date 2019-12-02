//
// Created by 赵鋆峰 on 2019/11/12.
//

#ifndef ROBODBMS_SP_MANAGER_H
#define ROBODBMS_SP_MANAGER_H


#include "../Constant.h"
#include "SP_Handle.h"

class SP_Manager {
public:
    static RC CreateStringPool(const char *fileName);

    static RC DestroyStringPool(const char *fileName);

    static RC OpenStringPool(const char *fileName, SP_Handle &spHandle);

    static RC CloseStringPool(SP_Handle &spHandle);

};


#endif //ROBODBMS_SP_MANAGER_H
