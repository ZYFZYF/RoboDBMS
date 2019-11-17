//
// Created by 赵鋆峰 on 2019/11/16.
//

#ifndef ROBODBMS_IX_INDEXHANDLE_H
#define ROBODBMS_IX_INDEXHANDLE_H


#include "../def.h"
#include "../RM/RM_RID.h"
#include "../PF/PF_FileHandle.h"
#include "IX_Manager.h"

#define IX_BPLUS_TREE_NODE_SIZE sizeof(IX_BPlusTreeNode)

typedef PageNum BPlusTreeNodePointer

struct IX_BPlusTreeNode {
    bool isLeaf;
    bool isRoot;
    int keyNum;
    BPlusTreeNodePointer prev, next, father;
};

class IX_IndexHandle {
    friend class IX_Manager;

public:
    IX_IndexHandle();                             // Constructor
    ~IX_IndexHandle();                             // Destructor
    RC InsertEntry(void *key, const RM_RID &value);  // Insert new index entry
    RC DeleteEntry(void *key, const RM_RID &value);  // Delete index entry
    RC ForcePages();                             // Copy index to disk
private:
    PF_FileHandle pfFileHandle;
    IX_FileHeader ixFileHeader;

    RC Find(void *key, const RM_RID &value, bool modify, BPlusTreeNodePointer &bPlusTreeNodePointer);

    RC BinarySearch(BPlusTreeNodePointer cur, void *key, const RM_RID &value, int &index);

    RC Split(BPlusTreeNodePointer cur);

    RC Insert(BPlusTreeNodePointer cur, void *key, const RM_RID &value);

    RC Resort(BPlusTreeNodePointer left, BPlusTreeNodePointer right);

    RC Redistribute(BPlusTreeNodePointer cur);

    RC Delete(BPlusTreeNodePointer cur, void *key, const RM_RID &value);

    bool Campare(CompOp compOp, void *keyLeft, void *valueLeft, void *keyRight, void *valueRight);
};


#endif //ROBODBMS_IX_INDEXHANDLE_H
