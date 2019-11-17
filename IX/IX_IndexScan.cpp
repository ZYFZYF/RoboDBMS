//
// Created by 赵鋆峰 on 2019/11/16.
//

#include <cstdlib>
#include <cstring>
#include "IX_IndexScan.h"
#include "def.h"
#include "../utils/Utils.h"

IX_IndexScan::IX_IndexScan() {

}

IX_IndexScan::~IX_IndexScan() {

}

RC IX_IndexScan::OpenScan(const IX_IndexHandle &indexHandle, CompOp compOp, void *value, ClientHint pinHint) {
    this->ixIndexHandle = const_cast<IX_IndexHandle *>(&indexHandle);
    this->compareOp = compOp;
    this->compareKey = (void *) malloc(ixIndexHandle->ixFileHeader.attrLength);
    memcpy(compareKey, value, ixIndexHandle->ixFileHeader.attrLength);
    currentKey = (void *) malloc(ixIndexHandle->ixFileHeader.attrLength);
    isFirst = false;
}

RC IX_IndexScan::GetNextEntry(RM_RID &rid) {
    //第一次返回的是第一个满足条件的
    if (isFirst) {
        isFirst = false;
        RM_RID minRID(MIN_PAGE_NUM, MIN_SLOT_NUM);
        RM_RID maxRID(MAX_PAGE_NUM, MAX_SLOT_NUM);
        switch (compareOp) {
            case NO_OP:
                ixIndexHandle->FindFirstEntry(currentLeaf, currentIndex, currentKey);
            case NE_OP:
                ixIndexHandle->FindFirstEntry(currentLeaf, currentIndex, currentKey);
            case LT_OP:
                ixIndexHandle->FindFirstEntry(currentLeaf, currentIndex, currentKey);
            case LE_OP:
                ixIndexHandle->FindFirstEntry(currentLeaf, currentIndex, currentKey);
            case EQ_OP:
                ixIndexHandle->Find(compareKey, &minRID, false, currentLeaf, currentIndex);
            case GT_OP:
                ixIndexHandle->Find(compareKey, &maxRID, false, currentLeaf, currentIndex);
            case GE_OP:
                ixIndexHandle->Find(compareKey, &minRID, false, currentLeaf, currentIndex);
        }
    } else {
        TRY(ixIndexHandle->GetNextEntry(currentLeaf, currentIndex, currentKey));
    }
    //如果是不等于，可能需要多跳几次
    if (compareOp == NE_OP) {
        while (!Utils::Compare(currentKey, compareKey, ixIndexHandle->ixFileHeader.attrType,
                               ixIndexHandle->ixFileHeader.attrLength, compareOp)) {
            TRY(ixIndexHandle->GetNextEntry(currentLeaf, currentIndex, currentKey));
        }
    }
    //如果这时候还不满足，那么后面的更不可能满足，扫描结束
    if (!Utils::Compare(currentKey, compareKey, ixIndexHandle->ixFileHeader.attrType,
                        ixIndexHandle->ixFileHeader.attrLength, compareOp)) {
        return IX_EOF;
    }
    //拿到RID的值
    TRY(ixIndexHandle->GetEntryValue(currentLeaf, currentIndex, &rid));
    return OK_RC;
}

RC IX_IndexScan::CloseScan() {
    return PF_NOBUF;
}
