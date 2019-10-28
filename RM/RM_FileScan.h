//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBODBMS_RM_FILESCAN_H
#define ROBODBMS_RM_FILESCAN_H


#include "RM_FileHandle.h"

class RM_FileScan {
public:
    RM_FileScan();

    ~RM_FileScan();

    RC OpenScan(const RM_FileHandle &fileHandle,
                AttrType attrType,
                int attrLength,
                int attrOffset,
                CompOp compOp,
                void *value,
                ClientHint pinHint = NO_HINT); // Initialize a file scan
    RC GetNextRec(RM_Record &rec);               // Get next matching record
    RC CloseScan();                             // Close the scan
};


#endif //ROBODBMS_RM_FILESCAN_H
