//
// Created by 赵鋆峰 on 2019/11/16.
//

#ifndef ROBODBMS_IX_INDEXHANDLE_H
#define ROBODBMS_IX_INDEXHANDLE_H


#include "../def.h"
#include "../RM/RM_RID.h"

class IX_IndexHandle {
public:
    IX_IndexHandle();                             // Constructor
    ~IX_IndexHandle();                             // Destructor
    RC InsertEntry(void *pData, const RM_RID &rid);  // Insert new index entry
    RC DeleteEntry(void *pData, const RM_RID &rid);  // Delete index entry
    RC ForcePages();                             // Copy index to disk
};


#endif //ROBODBMS_IX_INDEXHANDLE_H
