//
// Created by 赵鋆峰 on 2019/11/16.
//

#ifndef ROBODBMS_IX_MANAGER_H
#define ROBODBMS_IX_MANAGER_H


#include <string>
#include "../PF/PF_Manager.h"
#include "IX_IndexHandle.h"

#define IX_FILE_HEADER_SIZE sizeof(IX_FileHeader)

class IX_Manager {

public:
    static IX_Manager &Instance();

    ~IX_Manager();

    RC CreateIndex(const char *fileName,
                   int indexNo,
                   AttrType attrType,
                   int attrLength);

    RC DestroyIndex(const char *fileName,
                    int indexNo);

    RC OpenIndex(const char *fileName,
                 int indexNo,
                 IX_IndexHandle &indexHandle);

    RC CloseIndex(IX_IndexHandle &indexHandle);

private:
    IX_Manager();

    PF_Manager pfManager;
};


#endif //ROBODBMS_IX_MANAGER_H
