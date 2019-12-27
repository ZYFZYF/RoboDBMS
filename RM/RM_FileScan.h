//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBODBMS_RM_FILESCAN_H
#define ROBODBMS_RM_FILESCAN_H


#include "RM_FileHandle.h"
#include "../Attr.h"

#define BEGIN_SCAN  -1 //default slot number before scan begins

class RM_FileScan {
public:
    RM_FileScan();

    ~RM_FileScan();

    //默认是一个不需要比较的scan，遍历全部的记录
    RC OpenScan(const RM_FileHandle &rmFileHandle,
                AttrType attrType = INT,
                int attrLength = 0,
                int attrOffset = 0,
                Operator compOp = NO_OP,
                void *value = nullptr);

    RC GetNextRec(RM_Record &rec);

    RC CloseScan();

private:
    RC GetRecordNumOnPage(PF_PageHandle &pfPageHandle, int &recordNum);

    RM_FileHandle *rmFileHandle;

    int attrOffset;
    int attrLength;
    void *value;
    AttrType attrType;
    Operator compOp;

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
