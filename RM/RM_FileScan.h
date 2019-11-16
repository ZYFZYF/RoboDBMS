//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBODBMS_RM_FILESCAN_H
#define ROBODBMS_RM_FILESCAN_H


#include "RM_FileHandle.h"

#define BEGIN_SCAN  -1 //default slot number before scan begins

class RM_FileScan {
public:
    RM_FileScan();

    ~RM_FileScan();

    RC OpenScan(const RM_FileHandle &rmFileHandle,
                AttrType attrType,
                int attrLength,
                int attrOffset,
                CompOp compOp,
                void *value,
                ClientHint pinHint = NO_HINT); // Initialize a file scan
    RC GetNextRec(RM_Record &rec);               // Get next matching record
    RC CloseScan();                             // Close the scan
private:
    RC GetRecordNumOnPage(PF_PageHandle &pfPageHandle, int &recordNum);

    RM_FileHandle *rmFileHandle;

    int attrOffset;
    int attrLength;
    void *value;
    AttrType attrType;
    CompOp compOp;

    bool openScan;
    bool scanEnded;
    bool useNextPage;
    bool hasPagePinned;
    bool initializedValue;
    PageNum pageNum;
    SlotNum slotNum;
    PF_PageHandle currentPH;
    int numTotalOnPage;
    int numSeenOnPage;

};


#endif //ROBODBMS_RM_FILESCAN_H
