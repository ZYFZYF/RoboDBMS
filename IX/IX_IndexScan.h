//
// Created by 赵鋆峰 on 2019/11/16.
//

#ifndef ROBODBMS_IX_INDEXSCAN_H
#define ROBODBMS_IX_INDEXSCAN_H


#include "../def.h"
#include "IX_IndexHandle.h"

class IX_IndexScan {
public:
    IX_IndexScan();                                 // Constructor
    ~IX_IndexScan();                                 // Destructor
    RC OpenScan(const IX_IndexHandle &indexHandle, // Initialize index scan
                CompOp compOp,
                void *value,
                ClientHint pinHint = NO_HINT);

    RC GetNextEntry(RM_RID &rid);                         // Get next matching entry
    RC CloseScan();                                 // Terminate index scan
private:
    CompOp compareOp;
    IX_IndexHandle *ixIndexHandle;
    void *compareKey;
    BPlusTreeNodePointer currentLeaf;
    int currentIndex;
    void *currentKey;
    bool isFirst;
    bool isOpen;
};


#endif //ROBODBMS_IX_INDEXSCAN_H
