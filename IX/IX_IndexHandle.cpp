//
// Created by 赵鋆峰 on 2019/11/16.
//

#include "IX_IndexHandle.h"
#include "../utils/Utils.h"
#include "def.h"

IX_IndexHandle::IX_IndexHandle() {

}

IX_IndexHandle::~IX_IndexHandle() {

}

RC IX_IndexHandle::InsertEntry(void *key, const RM_RID &value) {
    return PF_NOBUF;
}

RC IX_IndexHandle::DeleteEntry(void *key, const RM_RID &value) {
    return PF_NOBUF;
}

RC IX_IndexHandle::ForcePages() {
    return PF_NOBUF;
}

RC IX_IndexHandle::Find(void *key, const RM_RID &value, bool modify, BPlusTreeNodePointer &bPlusTreeNodePointer) {
    return PF_NOBUF;
}

RC IX_IndexHandle::BinarySearch(BPlusTreeNodePointer cur, void *key, const RM_RID &value, int &index) {
    char *data;
    PF_PageHandle pfPageHandle;
    TRY(pfFileHandle.GetThisPage(cur, pfPageHandle));
    TRY(pfPageHandle.GetData(data));
    auto ixBPlusTreeNode = (IX_BPlusTreeNode *) data;
    int l = 0, r = ixBPlusTreeNode->keyNum;
    if (Campare(LT_OP, key, (void *) &value, data + ixFileHeader.firstKeyOffset + l * ixFileHeader.attrLength,
                data + ixFileHeader.firstValueOffset + VALUE_LENGTH)) {
        index = l;
        return OK_RC;
    }
    if (Campare(LT_OP, key, (void *) &value, data + ixFileHeader.firstKeyOffset + l * ixFileHeader.attrLength,
                data + ixFileHeader.firstValueOffset + VALUE_LENGTH)) {
        index = l;
        return OK_RC;
    }
}

RC IX_IndexHandle::Split(BPlusTreeNodePointer cur) {
    return PF_NOBUF;
}

RC IX_IndexHandle::Insert(BPlusTreeNodePointer cur, void *key, const RM_RID &value) {
    return PF_NOBUF;
}

RC IX_IndexHandle::Resort(BPlusTreeNodePointer left, BPlusTreeNodePointer right) {
    return PF_NOBUF;
}

RC IX_IndexHandle::Redistribute(BPlusTreeNodePointer cur) {
    return PF_NOBUF;
}

RC IX_IndexHandle::Delete(BPlusTreeNodePointer cur, void *key, const RM_RID &value) {
    return PF_NOBUF;
}

bool IX_IndexHandle::Campare(CompOp compOp, void *keyLeft, void *valueLeft, void *keyRight, void *valueRight) {
    RM_RID left = *(RM_RID *) valueLeft, right = *(RM_RID *) valueRight;
    bool keyEqual = Utils::Compare(keyLeft, keyRight, ixFileHeader.attrType, ixFileHeader.attrLength, EQ_OP);
    switch (compOp) {
        case NO_OP:
            return true;
        case EQ_OP:
            return Utils::Compare(keyLeft, keyRight, ixFileHeader.attrType, ixFileHeader.attrLength, EQ_OP) &&
                   left == right;
        case LT_OP:
            if (keyEqual)return left < right;
            else return Utils::Compare(keyLeft, keyRight, ixFileHeader.attrType, ixFileHeader.attrLength, LT_OP);
        case LE_OP:
            if (keyEqual)return left <= right;
            else return Utils::Compare(keyLeft, keyRight, ixFileHeader.attrType, ixFileHeader.attrLength, LE_OP);
        case NE_OP:
            return !Campare(EQ_OP, keyLeft, valueLeft, keyRight, valueRight);
        case GT_OP:
            return !Campare(LE_OP, keyLeft, valueLeft, keyRight, valueRight);
        case GE_OP:
            return !Campare(LT_OP, keyLeft, valueLeft, keyRight, valueRight);
    }
}
