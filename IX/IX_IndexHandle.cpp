//
// Created by 赵鋆峰 on 2019/11/16.
//

#include <cstring>
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

RC IX_IndexHandle::Find(void *key, void *value, bool modify, BPlusTreeNodePointer &bPlusTreeNodePointer) {
    return PF_NOBUF;
}

//记住，只要调用过GetThisPage或者AllocatePage或者GetThisPageData，就一定要unpin掉
RC IX_IndexHandle::BinarySearch(BPlusTreeNodePointer cur, void *key, void *value, int &index) {
    char *data;
    TRY(pfFileHandle.GetThisPageData(cur, data));
    auto ixBPlusTreeNode = (IX_BPlusTreeNode *) data;
    int l = 0, r = ixBPlusTreeNode->keyNum;
    if (Campare(LT_OP, key, value, GetKeyAt(data, l), GetValueAt(data, l))) {
        index = l;
        goto CLEAN_UP;
    }
    if (Campare(LT_OP, key, value, GetKeyAt(data, r - 1), GetValueAt(data, r - 1))) {
        index = r - 1;
        goto CLEAN_UP;
    }
    while (l < r - 1) {
        int mid = (l + r) >> 1;
        if (Campare(GT_OP, key, value, GetKeyAt(data, mid), GetValueAt(data, mid))) {
            r = mid;
        } else {
            l = mid;
        }
    }
    index = l;
    CLEAN_UP:
    TRY(pfFileHandle.UnpinPage(cur));
    return OK_RC;
}

RC IX_IndexHandle::Split(BPlusTreeNodePointer cur) {
    //先拿当前用到的指针什么的
    char *curPageStart;
    TRY(pfFileHandle.GetThisPageData(cur, curPageStart));
    auto curTreeNode = (IX_BPlusTreeNode *) curPageStart;
    //先分配一个新的出来，把用到的指针什么的拿到
    BPlusTreeNodePointer temp;
    PF_PageHandle pfPageHandle;
    TRY(pfFileHandle.AllocatePage(pfPageHandle));
    TRY(pfPageHandle.GetPageNum(temp));
    char *tempPageStart;
    TRY(pfPageHandle.GetData(tempPageStart));
    auto tempTreeNode = (IX_BPlusTreeNode *) tempPageStart;
    //开始划分，把一半的数据拷贝到新开的这个节点上
    int mid = ixFileHeader.maxKeyNum >> 1;
    tempTreeNode->isLeaf = curTreeNode->isLeaf;
    tempTreeNode->keyNum = ixFileHeader.maxKeyNum - mid;
    for (int i = mid; i < ixFileHeader.maxKeyNum; i++) {
        SetChildAt(tempPageStart, i - mid, GetChildAt(curPageStart, i));
        SetKeyAt(tempPageStart, i - mid, GetKeyAt(curPageStart, i));
        SetValueAt(tempPageStart, i - mid, GetValueAt(curPageStart, i));
        BPlusTreeNodePointer child = *(BPlusTreeNodePointer *) GetChildAt(tempPageStart, i - mid);
        char *childPageStart;
        TRY(pfFileHandle.GetThisPageData(child, childPageStart));
        auto childTreeNode = (IX_BPlusTreeNode *) childPageStart;
        childTreeNode->father = temp;
        TRY(pfFileHandle.MarkDirty(child));
        TRY(pfFileHandle.UnpinPage(child));
    }
    //改变当前节点的keyNum
    curTreeNode->keyNum = mid;
    if (curTreeNode->isRoot) {
        //当前点是根的话，还要再分配一个新节点当根，并且指向这两个节点
        BPlusTreeNodePointer root;
        TRY(pfFileHandle.AllocatePage(pfPageHandle));
        TRY(pfPageHandle.GetPageNum(root));
        char *rootPageStart;
        TRY(pfPageHandle.GetData(rootPageStart));
        auto rootTreeNode = (IX_BPlusTreeNode *) rootPageStart;
        rootTreeNode->isRoot = true;
        rootTreeNode->keyNum = 2;
        SetKeyAt(rootPageStart, 0, GetKeyAt(curPageStart, 0));
        SetValueAt(rootPageStart, 0, GetValueAt(curPageStart, 0));
        SetChildAt(rootPageStart, 0, &cur);
        SetKeyAt(rootPageStart, 1, GetKeyAt(curPageStart, 0));
        SetValueAt(rootPageStart, 1, GetValueAt(curPageStart, 0));
        SetChildAt(rootPageStart, 1, &temp);
        curTreeNode->father = tempTreeNode->father = root;
        curTreeNode->isRoot = false;
        if (curTreeNode->isLeaf) {
            curTreeNode->next = temp;
            tempTreeNode->prev = cur;
        }
        TRY(pfFileHandle.MarkDirty(root));
        TRY(pfFileHandle.MarkDirty(temp));
        TRY(pfFileHandle.MarkDirty(cur));
        TRY(pfFileHandle.UnpinPage(root));
        TRY(pfFileHandle.UnpinPage(temp));
        TRY(pfFileHandle.UnpinPage(cur));
    } else {
        tempTreeNode->father = curTreeNode->father;
        TRY(pfFileHandle.MarkDirty(temp));
        TRY(pfFileHandle.MarkDirty(cur));
        TRY(pfFileHandle.UnpinPage(temp));
        TRY(pfFileHandle.UnpinPage(cur));
        Insert(curTreeNode->father, GetKeyAt(curPageStart, mid), GetValueAt(curPageStart, mid), temp);
    }
}

RC IX_IndexHandle::Insert(BPlusTreeNodePointer cur, void *key, void *value, BPlusTreeNodePointer child) {
    return PF_NOBUF;
}

RC IX_IndexHandle::Resort(BPlusTreeNodePointer left, BPlusTreeNodePointer right) {
    return PF_NOBUF;
}

RC IX_IndexHandle::Redistribute(BPlusTreeNodePointer cur) {
    return PF_NOBUF;
}

RC IX_IndexHandle::Delete(BPlusTreeNodePointer cur, void *key, void *value) {
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

void *IX_IndexHandle::GetKeyAt(char *pageStart, int index) {
    return pageStart + ixFileHeader.firstKeyOffset + ixFileHeader.attrLength * index;
}

void *IX_IndexHandle::GetValueAt(char *pageStart, int index) {
    return pageStart + ixFileHeader.firstValueOffset + VALUE_LENGTH * index;
}

void *IX_IndexHandle::GetChildAt(char *pageStart, int index) {
    return pageStart + ixFileHeader.firstChildOffset + CHILD_LENGTH * index;
}

void IX_IndexHandle::SetKeyAt(char *pageStart, int index, void *key) {
    memcpy(GetKeyAt(pageStart, index), key, ixFileHeader.attrLength);
}

void IX_IndexHandle::SetValueAt(char *pageStart, int index, void *value) {
    memcpy(GetValueAt(pageStart, index), value, VALUE_LENGTH);
}

void IX_IndexHandle::SetChildAt(char *pageStart, int index, void *child) {
    memcpy(GetChildAt(pageStart, index), child, CHILD_LENGTH);
}

