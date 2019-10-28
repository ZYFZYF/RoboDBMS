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

    RC OpenScan(const RM_FileHandle &fileHandle,
                AttrType attrType,
                int attrLength,
                int attrOffset,
                CompOp compOp,
                void *value,
                ClientHint pinHint = NO_HINT); // Initialize a file scan
    RC GetNextRec(RM_Record &rec);               // Get next matching record
    RC CloseScan();                             // Close the scan
private:
    // Retrieves the number of records on ph's page, and returns it in
    // numRecords
    RC GetNumRecOnPage(PF_PageHandle &ph, int &numRecords);


    bool openScan; // whether this instance is currently a valid, open scan

    // save the parameters of the scan:
    RM_FileHandle *fileHandle;

    bool (*comparator)(void *, void *, AttrType, int);

    int attrOffset;
    int attrLength;
    void *value;
    AttrType attrType;
    CompOp compOp;

    // whether the scan has ended or not. This dictages whether to unpin the
    // page that the scan is on (currentPH)
    bool scanEnded;

    // The current state of the scan. currentPH is the page that's pinned
    PageNum scanPage;
    SlotNum scanSlot;
    PF_PageHandle currentPH;
    // Dictates whether to seek a record on the same page, or unpin it and
    // seek a record on the following page
    int numRecOnPage;
    int numSeenOnPage;
    bool useNextPage;
    bool hasPagePinned;
    bool initializedValue;
};


#endif //ROBODBMS_RM_FILESCAN_H
