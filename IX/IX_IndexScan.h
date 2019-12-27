//
// Created by 赵鋆峰 on 2019/11/16.
//

#ifndef ROBODBMS_IX_INDEXSCAN_H
#define ROBODBMS_IX_INDEXSCAN_H


#include "../Constant.h"
#include "IX_IndexHandle.h"

class IX_IndexScan {
public:
    IX_IndexScan();

    ~IX_IndexScan();

    RC OpenScan(const IX_IndexHandle &indexHandle,
                Operator compOp,
                void *value);

    RC GetNextEntry(RM_RID &rid);

    RC CloseScan();

    void *getCurrentKey();

private:
    Operator compareOp;
    IX_IndexHandle *ixIndexHandle;
    void *compareKey;
    BPlusTreeNodePointer currentLeaf;
    int currentIndex;
    void *currentKey;
    bool isFirst;
    bool isOpen;
};


#endif //ROBODBMS_IX_INDEXSCAN_H
