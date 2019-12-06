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

    ~IX_Manager();                             // Destructor
    RC CreateIndex(const char *fileName,          // Create new index
                   int indexNo,
                   AttrType attrType,
                   int attrLength);

    RC DestroyIndex(const char *fileName,          // Destroy index
                    int indexNo);

    RC OpenIndex(const char *fileName,          // Open index
                 int indexNo,
                 IX_IndexHandle &indexHandle);

    RC CloseIndex(IX_IndexHandle &indexHandle);  // Close index
private:
    IX_Manager();              // Constructor
    PF_Manager pfManager;
};


#endif //ROBODBMS_IX_MANAGER_H
