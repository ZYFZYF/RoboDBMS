//
// Created by 赵鋆峰 on 2019/11/16.
//

#include <cstring>
#include <vector>
#include "IX_IndexHandle.h"
#include "../utils/Utils.h"
#include "def.h"

IX_IndexHandle::IX_IndexHandle() {

}

IX_IndexHandle::~IX_IndexHandle() {

}

RC IX_IndexHandle::InsertEntry(void *key, const RM_RID &value) {
    BPlusTreeNodePointer leaf;
    void *temp = const_cast<RM_RID *>(&value);
    int index;
    Find(key, temp, true, leaf, index);
    char *leafPageStart;
    TRY(pfFileHandle.GetThisPageData(leaf, leafPageStart));
    auto leafTreeNode = (IX_BPlusTreeNode *) leafPageStart;
    if (Compare(EQ_OP, key, temp, GetKeyAt(leafPageStart, index), GetValueAt(leafPageStart, index))) {
        return IX_ALREADY_IN_BTREE;
    }
    TRY(Insert(leaf, key, temp, NULL_NODE));
    int keyNum = leafTreeNode->keyNum;
    int myKeys[keyNum];
    RM_RID myValues[keyNum];
    BPlusTreeNodePointer myChilds[keyNum];
    for (int i = 0; i < leafTreeNode->keyNum; i++) {
        myKeys[i] = *(int *) GetKeyAt(leafPageStart, i);
        myValues[i] = *(RM_RID *) GetValueAt(leafPageStart, i);
        myChilds[i] = *(BPlusTreeNodePointer *) GetChildAt(leafPageStart, i);
    }
    TRY(pfFileHandle.UnpinPage(leaf));
    return OK_RC;
}

RC IX_IndexHandle::DeleteEntry(void *key, const RM_RID &value) {
    BPlusTreeNodePointer leaf;
    void *temp = const_cast<RM_RID *>(&value);
    int index;
    Find(key, temp, false, leaf, index);
    char *leafPageStart;
    TRY(pfFileHandle.GetThisPageData(leaf, leafPageStart));
    auto leafTreeNode = (IX_BPlusTreeNode *) leafPageStart;
    if (Compare(NE_OP, key, temp, GetKeyAt(leafPageStart, index), GetValueAt(leafPageStart, index))) {
        return IX_ALREADY_NOT_IN_BTREE;
    }
    TRY(Delete(leaf, key, temp));
    TRY(pfFileHandle.UnpinPage(leaf));
    return OK_RC;
}

RC IX_IndexHandle::ForcePages() {
    return PF_NOBUF;
}


RC IX_IndexHandle::FindFirstEntry(BPlusTreeNodePointer &bPlusTreeNodePointer, int &index, void *actualKey) {
    BPlusTreeNodePointer cur = ixFileHeader.rootPageNum;
    char *curPageStart;
    while (true) {
        BPlusTreeNodePointer temp = cur;
        TRY(pfFileHandle.GetThisPageData(cur, curPageStart));
        auto curTreeNode = (IX_BPlusTreeNode *) curPageStart;
        if (curTreeNode->isLeaf) {
            break;
        }
        cur = *(BPlusTreeNodePointer *) GetChildAt(curPageStart, 0);
        TRY(pfFileHandle.UnpinPage(temp));
    }
    bPlusTreeNodePointer = cur;
    index = 0;
    memcpy(actualKey, GetKeyAt(curPageStart, index), ixFileHeader.attrLength);
    int mykey = *(int *) actualKey;
    TRY(pfFileHandle.UnpinPage(cur));
    return OK_RC;
}

RC IX_IndexHandle::Find(void *key, void *value, bool modify, BPlusTreeNodePointer &bPlusTreeNodePointer, int &index,
                        void *actualKey) {
    char *curPageStart;
    BPlusTreeNodePointer cur = ixFileHeader.rootPageNum;
    while (true) {
        BPlusTreeNodePointer temp = cur;
        TRY(pfFileHandle.GetThisPageData(cur, curPageStart));
        auto curTreeNode = (IX_BPlusTreeNode *) curPageStart;
        if (curTreeNode->isLeaf) {
            break;
        }
        if (Compare(LT_OP, key, value, GetKeyAt(curPageStart, 0), GetValueAt(curPageStart, 0))) {
            if (modify) {
                SetKeyAt(curPageStart, 0, key);
                SetValueAt(curPageStart, 0, value);
                TRY(pfFileHandle.MarkDirty(temp));
            }
            cur = *(BPlusTreeNodePointer *) GetChildAt(curPageStart, 0);
            TRY(pfFileHandle.UnpinPage(temp));
        } else {
            BinarySearch(cur, key, value, index);
            cur = *(BPlusTreeNodePointer *) GetChildAt(curPageStart, index);
            TRY(pfFileHandle.UnpinPage(temp));
        }
    }
    bPlusTreeNodePointer = cur;
    BinarySearch(cur, key, value, index);
    if (actualKey != nullptr) {
        memcpy(actualKey, GetKeyAt(curPageStart, index), ixFileHeader.attrLength);
    }
    auto curTreeNode = (IX_BPlusTreeNode *) curPageStart;
    int keyNum = curTreeNode->keyNum;
    int myKeys[keyNum];
    RM_RID myValues[keyNum];
    BPlusTreeNodePointer myChilds[keyNum];
    for (int i = 0; i < curTreeNode->keyNum; i++) {
        myKeys[i] = *(int *) GetKeyAt(curPageStart, i);
        myValues[i] = *(RM_RID *) GetValueAt(curPageStart, i);
        myChilds[i] = *(BPlusTreeNodePointer *) GetChildAt(curPageStart, i);
    }
    TRY(pfFileHandle.UnpinPage(cur));
    return OK_RC;
}

//记住，只要调用过GetThisPage或者AllocatePage或者GetThisPageData，就一定要unpin掉
//返回的是最后一个<=key的index的下标
RC IX_IndexHandle::BinarySearch(BPlusTreeNodePointer cur, void *key, void *value, int &index) {
    int actualKey, queryKey, compareKey;
    char *data;
    TRY(pfFileHandle.GetThisPageData(cur, data));
    auto ixBPlusTreeNode = (IX_BPlusTreeNode *) data;
    int l = 0, r = ixBPlusTreeNode->keyNum;
    //先判断两个边界
    if (ixBPlusTreeNode->keyNum > 0 && Compare(LT_OP, key, value, GetKeyAt(data, l), GetValueAt(data, l))) {
        index = l;
        goto CLEAN_UP;
    }
    if (ixBPlusTreeNode->keyNum > 0 && Compare(GE_OP, key, value, GetKeyAt(data, r - 1), GetValueAt(data, r - 1))) {
        index = r - 1;
        goto CLEAN_UP;
    }
    queryKey = *(int *) key;
    while (l < r - 1) {
        int mid = (l + r) >> 1;
        compareKey = *(int *) GetKeyAt(data, mid);
        if (Compare(LT_OP, key, value, GetKeyAt(data, mid), GetValueAt(data, mid))) {
            r = mid;
        } else {
            l = mid;
        }
    }
    index = l;

    CLEAN_UP:
    actualKey = *(int *) key;
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
        //如果该节点不是叶子节点的话，才更新他的儿子的父指针
        if (child != NULL_NODE) {
            char *childPageStart;
            TRY(pfFileHandle.GetThisPageData(child, childPageStart));
            auto childTreeNode = (IX_BPlusTreeNode *) childPageStart;
            childTreeNode->father = temp;
            TRY(pfFileHandle.MarkDirty(child));
            TRY(pfFileHandle.UnpinPage(child));
        }
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
        SetKeyAt(rootPageStart, 1, GetKeyAt(tempPageStart, 0));
        SetValueAt(rootPageStart, 1, GetValueAt(tempPageStart, 0));
        SetChildAt(rootPageStart, 1, &temp);
        curTreeNode->father = tempTreeNode->father = root;
        curTreeNode->isRoot = false;
        if (curTreeNode->isLeaf) {
            curTreeNode->next = temp;
            tempTreeNode->prev = cur;
        }
        //修改文件头中存储的根页
        ixFileHeader.rootPageNum = root;
        headerChanged = true;
        TRY(pfFileHandle.MarkDirty(root));
        TRY(pfFileHandle.MarkDirty(temp));
        TRY(pfFileHandle.MarkDirty(cur));
        TRY(pfFileHandle.UnpinPage(root));
        TRY(pfFileHandle.UnpinPage(temp));
        TRY(pfFileHandle.UnpinPage(cur));
    } else {
        //否则给个新爸爸，并且递归插入到爸爸节点里
        tempTreeNode->father = curTreeNode->father;
        TRY(pfFileHandle.MarkDirty(temp));
        TRY(pfFileHandle.MarkDirty(cur));
        TRY(pfFileHandle.UnpinPage(temp));
        TRY(pfFileHandle.UnpinPage(cur));
        Insert(curTreeNode->father, GetKeyAt(curPageStart, mid), GetValueAt(curPageStart, mid), temp);
    }
    return OK_RC;
}

RC IX_IndexHandle::Insert(BPlusTreeNodePointer cur, void *key, void *value, BPlusTreeNodePointer child) {
    //先拿当前用到的指针什么的
    char *curPageStart;
    TRY(pfFileHandle.GetThisPageData(cur, curPageStart));
    auto curTreeNode = (IX_BPlusTreeNode *) curPageStart;
    //找到插入的位置
    int index = 0;
    if (curTreeNode->keyNum > 0 && Compare(GE_OP, key, value, GetKeyAt(curPageStart, 0), GetValueAt(curPageStart, 0))) {
        BinarySearch(cur, key, value, index);
        index++;
    }
    int actualKey = *(int *) key;
    //把后面的都往前挪
    for (int i = curTreeNode->keyNum; i > index; i--) {
        SetKeyAt(curPageStart, i, GetKeyAt(curPageStart, i - 1));
        SetValueAt(curPageStart, i, GetValueAt(curPageStart, i - 1));
        SetChildAt(curPageStart, i, GetChildAt(curPageStart, i - 1));
    }
    //把child放进来，如果是叶节点child可以是-1
    curTreeNode->keyNum++;
    SetKeyAt(curPageStart, index, key);
    SetValueAt(curPageStart, index, value);
    SetChildAt(curPageStart, index, &child);
    if (!curTreeNode->isLeaf) {
        //拿到要插入的child的相关指针
        char *childPageStart;
        TRY(pfFileHandle.GetThisPageData(child, childPageStart));
        auto childTreeNode = (IX_BPlusTreeNode *) childPageStart;
        //用第一个儿子判断插入的节点是不是叶子
        BPlusTreeNodePointer firstChild = *(BPlusTreeNodePointer *) GetChildAt(curPageStart, 0);
        char *firstChildPageStart;
        TRY(pfFileHandle.GetThisPageData(firstChild, firstChildPageStart));
        auto firstChildTreeNode = (IX_BPlusTreeNode *) firstChildPageStart;
        if (firstChildTreeNode->isLeaf) {
            //是的话要前后串起来
            if (index > 0) {
                auto prevChild = *(BPlusTreeNodePointer *) GetChildAt(curPageStart, index - 1);
                char *prevChildPageStart;
                TRY(pfFileHandle.GetThisPageData(prevChild, prevChildPageStart));
                auto prevChildTreeNode = (IX_BPlusTreeNode *) prevChildPageStart;
                auto nextChild = prevChildTreeNode->next;
                prevChildTreeNode->next = child;
                childTreeNode->prev = prevChild;
                childTreeNode->next = nextChild;
                if (nextChild != NULL_NODE) {
                    char *nextChildPageStart;
                    TRY(pfFileHandle.GetThisPageData(nextChild, nextChildPageStart));
                    auto nextChildTreeNode = (IX_BPlusTreeNode *) nextChildPageStart;
                    nextChildTreeNode->prev = child;
                    TRY(pfFileHandle.MarkDirty(nextChild));
                    TRY(pfFileHandle.UnpinPage(nextChild));
                }
                TRY(pfFileHandle.MarkDirty(prevChild));
                TRY(pfFileHandle.UnpinPage(prevChild));
            } else {
                childTreeNode->next = *(BPlusTreeNodePointer *) GetChildAt(curPageStart, 1);
            }
            TRY(pfFileHandle.MarkDirty(child));
            TRY(pfFileHandle.UnpinPage(child));
        } else {
            TRY(pfFileHandle.UnpinPage(child));
        }
        TRY(pfFileHandle.UnpinPage(firstChild));
    }
    TRY(pfFileHandle.MarkDirty(cur));
    TRY(pfFileHandle.UnpinPage(cur));
    if (curTreeNode->keyNum == ixFileHeader.maxKeyNum) {
        Split(cur);
    }
    return OK_RC;
}

RC IX_IndexHandle::Resort(BPlusTreeNodePointer left, BPlusTreeNodePointer right) {
    return PF_NOBUF;
}

RC IX_IndexHandle::Redistribute(BPlusTreeNodePointer cur) {
    return OK_RC;
}

RC IX_IndexHandle::Delete(BPlusTreeNodePointer cur, void *key, void *value) {
    //先拿当前用到的指针什么的
    char *curPageStart;
    TRY(pfFileHandle.GetThisPageData(cur, curPageStart));
    auto curTreeNode = (IX_BPlusTreeNode *) curPageStart;
    int index;
    BinarySearch(cur, key, value, index);
    //拿到要删除的节点的页
    auto deleteChild = *(BPlusTreeNodePointer *) GetChildAt(curPageStart, index);
    //把后面的往前挪
    for (int i = index; i < curTreeNode->keyNum - 1; i++) {
        SetKeyAt(curPageStart, i, GetKeyAt(curPageStart, i + 1));
        SetValueAt(curPageStart, i, GetValueAt(curPageStart, i + 1));
        SetChildAt(curPageStart, i, GetChildAt(curPageStart, i + 1));
    }
    curTreeNode->keyNum--;
    //如果他们孩子是叶子的话，需要处理孩子之间的链接关系
    if (!curTreeNode->isLeaf) {
        //用第一个儿子的isLeaf来看
        auto firstChild = *(BPlusTreeNodePointer *) GetChildAt(curPageStart, 0);
        char *firstChildPageStart;
        TRY(pfFileHandle.GetThisPageData(firstChild, firstChildPageStart));
        auto firstChildTreeNode = (IX_BPlusTreeNode *) firstChildPageStart;
        if (firstChildTreeNode->isLeaf) {
            char *deleteChildPageStart;
            TRY(pfFileHandle.GetThisPageData(deleteChild, deleteChildPageStart));
            auto deleteChildTreeNode = (IX_BPlusTreeNode *) deleteChildPageStart;
            BPlusTreeNodePointer prevChild = deleteChildTreeNode->prev;
            BPlusTreeNodePointer nextChild = deleteChildTreeNode->next;
            if (prevChild != NULL_NODE) {
                char *prevChildPageStart;
                TRY(pfFileHandle.GetThisPageData(prevChild, prevChildPageStart));
                auto prevChildTreeNode = (IX_BPlusTreeNode *) prevChildPageStart;
                prevChildTreeNode->next = nextChild;
                TRY(pfFileHandle.MarkDirty(prevChild));
                TRY(pfFileHandle.UnpinPage(prevChild));
            }
            if (nextChild != NULL_NODE) {
                char *nextChildPageStart;
                TRY(pfFileHandle.GetThisPageData(nextChild, nextChildPageStart));
                auto nextChildTreeNode = (IX_BPlusTreeNode *) nextChildPageStart;
                nextChildTreeNode->prev = prevChild;
                TRY(pfFileHandle.MarkDirty(nextChild));
                TRY(pfFileHandle.UnpinPage(nextChild));
            }
        }
        TRY(pfFileHandle.UnpinPage(firstChild));
    }
    //沿着父亲边修改一些key值(以及value值)
    if (index == 0 && !curTreeNode->isRoot) {
        BPlusTreeNodePointer temp = cur;
        IX_BPlusTreeNode *tempTreeNode = curTreeNode;
        //更新一路向上的节点的key值
        while (!tempTreeNode->isRoot) {
            char *fatherPageStart;
            BPlusTreeNodePointer father = tempTreeNode->father;
            TRY(pfFileHandle.GetThisPageData(father, fatherPageStart));
            auto fatherTreeNode = (IX_BPlusTreeNode *) fatherPageStart;
            if (*(BPlusTreeNodePointer *) GetChildAt(fatherPageStart, 0) == temp) {
                SetKeyAt(fatherPageStart, 0, GetKeyAt(curPageStart, 0));
                SetValueAt(fatherPageStart, 0, GetValueAt(curPageStart, 0));
                temp = father;
                tempTreeNode = fatherTreeNode;
            } else {
                int childIndex;
                BinarySearch(father, key, value, childIndex);
                SetKeyAt(fatherPageStart, childIndex, GetKeyAt(curPageStart, 0));
                SetValueAt(fatherPageStart, childIndex, GetValueAt(curPageStart, 0));
                TRY(pfFileHandle.MarkDirty(father));
                TRY(pfFileHandle.UnpinPage(father));
                break;
            }
            TRY(pfFileHandle.MarkDirty(father));
            TRY(pfFileHandle.UnpinPage(father));
        }
    }
    if (curTreeNode->keyNum * 2 < ixFileHeader.maxKeyNum) {
        TRY(Redistribute(cur));
    }
    TRY(pfFileHandle.MarkDirty(cur));
    TRY(pfFileHandle.UnpinPage(cur));
    return OK_RC;
}

bool IX_IndexHandle::Compare(CompOp compOp, void *keyLeft, void *valueLeft, void *keyRight, void *valueRight) {
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
            return !Compare(EQ_OP, keyLeft, valueLeft, keyRight, valueRight);
        case GT_OP:
            return !Compare(LE_OP, keyLeft, valueLeft, keyRight, valueRight);
        case GE_OP:
            return !Compare(LT_OP, keyLeft, valueLeft, keyRight, valueRight);
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

RC IX_IndexHandle::GetNextEntry(BPlusTreeNodePointer &cur, int &index, void *actualKey) {
    //先拿当前用到的指针什么的
    char *curPageStart;
    TRY(pfFileHandle.GetThisPageData(cur, curPageStart));
    auto curTreeNode = (IX_BPlusTreeNode *) curPageStart;
    //如果当前已经是最后一个那么要跳到下一个叶子节点
    if (++index == curTreeNode->keyNum) {
        BPlusTreeNodePointer temp = cur;
        cur = curTreeNode->next;
        TRY(pfFileHandle.UnpinPage(temp));
        if (cur == NULL_NODE) {
            return IX_EOF;
        } else {
            TRY(pfFileHandle.GetThisPageData(cur, curPageStart));
            index = 0;
            memcpy(actualKey, GetKeyAt(curPageStart, index), ixFileHeader.attrLength);
            TRY(pfFileHandle.UnpinPage(cur));
            return OK_RC;
        }
    } else {
        memcpy(actualKey, GetKeyAt(curPageStart, index), ixFileHeader.attrLength);
        TRY(pfFileHandle.UnpinPage(cur));
        return OK_RC;
    }
}

RC IX_IndexHandle::GetEntryValue(BPlusTreeNodePointer cur, int index, void *value) {
    //先拿当前用到的指针什么的
    char *curPageStart;
    TRY(pfFileHandle.GetThisPageData(cur, curPageStart));
    auto curTreeNode = (IX_BPlusTreeNode *) curPageStart;
    //如果当前已经是最后一个那么要跳到下一个叶子节点
    memcpy(value, GetValueAt(curPageStart, index), VALUE_LENGTH);
    TRY(pfFileHandle.UnpinPage(cur));
    return OK_RC;
}
