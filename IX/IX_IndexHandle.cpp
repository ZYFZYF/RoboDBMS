//
// Created by 赵鋆峰 on 2019/11/16.
//

#include <cstring>
#include <vector>
#include "IX_IndexHandle.h"
#include "../utils/Utils.h"
#include "IX_Constant.h"

IX_IndexHandle::IX_IndexHandle() {

}

IX_IndexHandle::~IX_IndexHandle() {

}

RC IX_IndexHandle::InsertEntry(void *key, const RM_RID &value) {
    BPlusTreeNodePointer leaf;
    void *temp = const_cast<RM_RID *>(&value);
    int index;
    TRY(Find(key, temp, true, leaf, index));
    char *leafPageStart;
    TRY(pfFileHandle.GetThisPageData(leaf, leafPageStart));
    auto leafTreeNode = (IX_BPlusTreeNode *) leafPageStart;
    if (Compare(EQ_OP, key, temp, GetKeyAt(leafPageStart, index), GetValueAt(leafPageStart, index))) {
        TRY(pfFileHandle.UnpinPage(leaf));
        return IX_ALREADY_IN_BTREE;
    }
    TRY(Insert(leaf, key, temp, NULL_NODE));
    TRY(pfFileHandle.UnpinPage(leaf));
    return OK_RC;
}

RC IX_IndexHandle::DeleteEntry(void *key, const RM_RID &value) {
    BPlusTreeNodePointer leaf;
    void *temp = const_cast<RM_RID *>(&value);
    int index;
    TRY(Find(key, temp, false, leaf, index));
    char *leafPageStart;
    TRY(pfFileHandle.GetThisPageData(leaf, leafPageStart));
    if (Compare(NE_OP, key, temp, GetKeyAt(leafPageStart, index), GetValueAt(leafPageStart, index))) {
        TRY(pfFileHandle.UnpinPage(leaf));
        return IX_ALREADY_NOT_IN_BTREE;
    }
    TRY(pfFileHandle.UnpinPage(leaf));
    TRY(Delete(leaf, key, temp));
    return OK_RC;
}

RC IX_IndexHandle::ForcePages() {
    return PF_NOBUF;
}


RC IX_IndexHandle::FindFirstEntry(BPlusTreeNodePointer &bPlusTreeNodePointer, int &index, void *actualKey) {
    BPlusTreeNodePointer cur = ixFileHeader.rootPageNum;
    char *curPageStart;
    IX_BPlusTreeNode *curTreeNode;
    while (true) {
        BPlusTreeNodePointer temp = cur;
        TRY(pfFileHandle.GetThisPageData(cur, curPageStart));
        curTreeNode = (IX_BPlusTreeNode *) curPageStart;
        if (curTreeNode->isLeaf) {
            break;
        }
        cur = *(BPlusTreeNodePointer *) GetChildAt(curPageStart, 0);
        TRY(pfFileHandle.UnpinPage(temp));
    }
    bPlusTreeNodePointer = cur;
    index = 0;
    //有可能已经空了，这时候要返回IX_EOF
    if (index < curTreeNode->keyNum) {
        memcpy(actualKey, GetKeyAt(curPageStart, index), ixFileHeader.attrLength);
        TRY(pfFileHandle.UnpinPage(cur));
        return OK_RC;
    } else {
        TRY(pfFileHandle.UnpinPage(cur));
        return IX_EOF;
    }
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
    auto curTreeNode = (IX_BPlusTreeNode *) curPageStart;
    //判断找到的是不是合法的,如果是插入的话可以是不存在的index
    if (modify || index < curTreeNode->keyNum) {
        if (actualKey != nullptr) {
            memcpy(actualKey, GetKeyAt(curPageStart, index), ixFileHeader.attrLength);
        }
        TRY(pfFileHandle.UnpinPage(cur));
        return OK_RC;
    } else {
        TRY(pfFileHandle.UnpinPage(cur));
        return IX_EOF;
    }
}

//记住，只要调用过GetThisPage或者AllocatePage或者GetThisPageData，就一定要unpin掉
//返回的是最后一个<=key的index的下标
RC IX_IndexHandle::BinarySearch(BPlusTreeNodePointer cur, void *key, void *value, int &index) {
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
    while (l < r - 1) {
        int mid = (l + r) >> 1;
        if (Compare(LT_OP, key, value, GetKeyAt(data, mid), GetValueAt(data, mid))) {
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
    tempTreeNode->prev = tempTreeNode->next = NULL_NODE;
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
    //先拿到左右节点需要的指针
    char *leftPageStart, *rightPageStart;
    TRY(pfFileHandle.GetThisPageData(left, leftPageStart));
    TRY(pfFileHandle.GetThisPageData(right, rightPageStart));
    auto leftTreeNode = (IX_BPlusTreeNode *) leftPageStart;
    auto rightTreeNode = (IX_BPlusTreeNode *) rightPageStart;
    int totalKeyNum = leftTreeNode->keyNum + rightTreeNode->keyNum;
    //左边比较少的话就从右边往左边挪
    if (leftTreeNode->keyNum < rightTreeNode->keyNum) {
        int leftSize = totalKeyNum >> 1;
        //先把左边需要的挪过来
        int i = 0;
        for (; leftTreeNode->keyNum < leftSize; i++, leftTreeNode->keyNum++) {
            SetKeyAt(leftPageStart, leftTreeNode->keyNum, GetKeyAt(rightPageStart, i));
            SetValueAt(leftPageStart, leftTreeNode->keyNum, GetValueAt(rightPageStart, i));
            SetChildAt(leftPageStart, leftTreeNode->keyNum, GetChildAt(rightPageStart, i));
            BPlusTreeNodePointer child = *(BPlusTreeNodePointer *) GetChildAt(leftPageStart, leftTreeNode->keyNum);
            if (child != NULL_NODE) {
                char *childPageStart;
                TRY(pfFileHandle.GetThisPageData(child, childPageStart));
                auto childTreeNode = (IX_BPlusTreeNode *) childPageStart;
                childTreeNode->father = left;
                TRY(pfFileHandle.MarkDirty(child));
                TRY(pfFileHandle.UnpinPage(child));
            }
        }
        int j = 0;
        //再把右边的往自己的前面挪
        for (; i < rightTreeNode->keyNum; j++, i++) {
            SetKeyAt(rightPageStart, j, GetKeyAt(rightPageStart, i));
            SetValueAt(rightPageStart, j, GetValueAt(rightPageStart, i));
            SetChildAt(rightPageStart, j, GetChildAt(rightPageStart, i));
        }
        rightTreeNode->keyNum = totalKeyNum - leftSize;
    } else {
        int rightSize = totalKeyNum >> 1;
        //先把右边的往后挪，腾出位置来
        for (int i = rightSize - 1, j = rightTreeNode->keyNum - 1; j >= 0; i--, j--) {
            SetKeyAt(rightPageStart, i, GetKeyAt(rightPageStart, j));
            SetValueAt(rightPageStart, i, GetValueAt(rightPageStart, j));
            SetChildAt(rightPageStart, i, GetChildAt(rightPageStart, j));
        }
        //再把左边靠后的拷过去
        for (int i = leftTreeNode->keyNum - 1, j = rightSize - 1 - rightTreeNode->keyNum; j >= 0; i--, j--) {
            SetKeyAt(rightPageStart, j, GetKeyAt(leftPageStart, i));
            SetValueAt(rightPageStart, j, GetValueAt(leftPageStart, i));
            SetChildAt(rightPageStart, j, GetChildAt(leftPageStart, i));
            BPlusTreeNodePointer child = *(BPlusTreeNodePointer *) GetChildAt(rightPageStart, j);
            if (child != NULL_NODE) {
                char *childPageStart;
                TRY(pfFileHandle.GetThisPageData(child, childPageStart));
                auto childTreeNode = (IX_BPlusTreeNode *) childPageStart;
                childTreeNode->father = right;
                TRY(pfFileHandle.MarkDirty(child));
                TRY(pfFileHandle.UnpinPage(child));
            }
        }
        rightTreeNode->keyNum = rightSize;
        leftTreeNode->keyNum = totalKeyNum - rightSize;
    }
    TRY(pfFileHandle.MarkDirty(left));
    TRY(pfFileHandle.MarkDirty(right));
    TRY(pfFileHandle.UnpinPage(left));
    TRY(pfFileHandle.UnpinPage(right));
    return OK_RC;
}

//这种时候都是cur的节点数刚刚不够下限，所以需要一个就能拜托"濒死"状态
RC IX_IndexHandle::Redistribute(BPlusTreeNodePointer cur) {
    //先拿当前用到的指针什么的
    char *curPageStart;
    TRY(pfFileHandle.GetThisPageData(cur, curPageStart));
    auto curTreeNode = (IX_BPlusTreeNode *) curPageStart;
    if (curTreeNode->isRoot) {
        //根节点只有当只有一个儿子并且不是叶子的时候才重分布
        if (curTreeNode->keyNum == 1 && !curTreeNode->isLeaf) {
            BPlusTreeNodePointer newRoot = *(BPlusTreeNodePointer *) GetChildAt(curPageStart, 0);
            ixFileHeader.rootPageNum = newRoot;
            headerChanged = true;
            char *newRootPageStart;
            TRY(pfFileHandle.GetThisPageData(newRoot, newRootPageStart));
            auto newRootTreeNode = (IX_BPlusTreeNode *) newRootPageStart;
            newRootTreeNode->isRoot = true;
            TRY(pfFileHandle.MarkDirty(newRoot));
            TRY(pfFileHandle.UnpinPage(newRoot));
            TRY(pfFileHandle.UnpinPage(cur));
            TRY(pfFileHandle.DisposePage(cur));
            return OK_RC;
        } else {
            TRY(pfFileHandle.UnpinPage(cur));
            return OK_RC;
        }
    }

    BPlusTreeNodePointer father = curTreeNode->father;
    char *fatherPageStart;
    TRY(pfFileHandle.GetThisPageData(father, fatherPageStart));
    auto fatherTreeNode = (IX_BPlusTreeNode *) fatherPageStart;
    //找出cur在父亲中所处的位置
    int index;
    BinarySearch(father, GetKeyAt(curPageStart, 0), GetValueAt(curPageStart, 0), index);
    //尝试从右边挪一个过来
    if (index + 1 < fatherTreeNode->keyNum) {
        BPlusTreeNodePointer nextChild = *(BPlusTreeNodePointer *) GetChildAt(fatherPageStart, index + 1);
        char *nextChildPageStart;
        TRY(pfFileHandle.GetThisPageData(nextChild, nextChildPageStart));
        auto nextChildTreeNode = (IX_BPlusTreeNode *) nextChildPageStart;
        //判断去掉一个之后会不会"死亡"，不会的话就给我一个（做法上是索性两边平均，这样不会导致每次删一个都得借）
        if ((nextChildTreeNode->keyNum - 1) * 2 >= ixFileHeader.maxKeyNum) {
            TRY(Resort(cur, nextChild));
            //重新分配了之后要修改父亲的key
            SetKeyAt(fatherPageStart, index + 1, GetKeyAt(nextChildPageStart, 0));
            SetValueAt(fatherPageStart, index + 1, GetValueAt(nextChildPageStart, 0));
            TRY(pfFileHandle.MarkDirty(nextChild));
            TRY(pfFileHandle.MarkDirty(father));
            TRY(pfFileHandle.MarkDirty(cur));
            TRY(pfFileHandle.UnpinPage(nextChild));
            TRY(pfFileHandle.UnpinPage(father));
            TRY(pfFileHandle.UnpinPage(cur));
            return OK_RC;
        }
        TRY(pfFileHandle.MarkDirty(nextChild));
        TRY(pfFileHandle.UnpinPage(nextChild));
    }
    //尝试从左边挪一个过来
    if (index > 0) {
        BPlusTreeNodePointer prevChild = *(BPlusTreeNodePointer *) GetChildAt(fatherPageStart, index - 1);
        char *prevChildPageStart;
        TRY(pfFileHandle.GetThisPageData(prevChild, prevChildPageStart));
        auto prevChildTreeNode = (IX_BPlusTreeNode *) prevChildPageStart;
        //判断去掉一个之后会不会"死亡"，不会的话就给我一个（做法上是索性两边平均，这样不会导致每次删一个都得借）
        if ((prevChildTreeNode->keyNum - 1) * 2 >= ixFileHeader.maxKeyNum) {
            Resort(prevChild, cur);
            //重新分配了之后要修改父亲的key
            SetKeyAt(fatherPageStart, index, GetKeyAt(curPageStart, 0));
            SetValueAt(fatherPageStart, index, GetValueAt(curPageStart, 0));
            TRY(pfFileHandle.MarkDirty(prevChild));
            TRY(pfFileHandle.MarkDirty(father));
            TRY(pfFileHandle.MarkDirty(cur));
            TRY(pfFileHandle.UnpinPage(prevChild));
            TRY(pfFileHandle.UnpinPage(father));
            TRY(pfFileHandle.UnpinPage(cur));
            return OK_RC;
        }
        TRY(pfFileHandle.MarkDirty(prevChild));
        TRY(pfFileHandle.UnpinPage(prevChild));
    }
    //尝试和右边合并
    if (index + 1 < fatherTreeNode->keyNum) {
        BPlusTreeNodePointer nextChild = *(BPlusTreeNodePointer *) GetChildAt(fatherPageStart, index + 1);
        char *nextChildPageStart;
        TRY(pfFileHandle.GetThisPageData(nextChild, nextChildPageStart));
        auto nextChildTreeNode = (IX_BPlusTreeNode *) nextChildPageStart;
        for (int i = 0; i < nextChildTreeNode->keyNum; i++, curTreeNode->keyNum++) {
            SetKeyAt(curPageStart, curTreeNode->keyNum, GetKeyAt(nextChildPageStart, i));
            SetValueAt(curPageStart, curTreeNode->keyNum, GetValueAt(nextChildPageStart, i));
            SetChildAt(curPageStart, curTreeNode->keyNum, GetChildAt(nextChildPageStart, i));
            auto child = *(BPlusTreeNodePointer *) GetChildAt(curPageStart, curTreeNode->keyNum);
            if (child != NULL_NODE) {
                char *childPageStart;
                TRY(pfFileHandle.GetThisPageData(child, childPageStart));
                auto childTreeNode = (IX_BPlusTreeNode *) childPageStart;
                childTreeNode->father = cur;
                TRY(pfFileHandle.MarkDirty(child));
                TRY(pfFileHandle.UnpinPage(child));
            }
        }
        TRY(pfFileHandle.MarkDirty(nextChild));
        TRY(pfFileHandle.MarkDirty(father));
        TRY(pfFileHandle.MarkDirty(cur));
        TRY(pfFileHandle.UnpinPage(nextChild));
        TRY(pfFileHandle.UnpinPage(father));
        TRY(pfFileHandle.UnpinPage(cur));
        TRY(Delete(father, GetKeyAt(nextChildPageStart, 0), GetValueAt(nextChildPageStart, 0)));
        return OK_RC;
    }
    //尝试和左边合并
    if (index > 0) {
        BPlusTreeNodePointer prevChild = *(BPlusTreeNodePointer *) GetChildAt(fatherPageStart, index - 1);
        char *prevChildPageStart;
        TRY(pfFileHandle.GetThisPageData(prevChild, prevChildPageStart));
        auto prevChildTreeNode = (IX_BPlusTreeNode *) prevChildPageStart;
        for (int i = 0; i < curTreeNode->keyNum; i++, prevChildTreeNode->keyNum++) {
            SetKeyAt(prevChildPageStart, prevChildTreeNode->keyNum, GetKeyAt(curPageStart, i));
            SetValueAt(prevChildPageStart, prevChildTreeNode->keyNum, GetValueAt(curPageStart, i));
            SetChildAt(prevChildPageStart, prevChildTreeNode->keyNum, GetChildAt(curPageStart, i));
            auto child = *(BPlusTreeNodePointer *) GetChildAt(prevChildPageStart, prevChildTreeNode->keyNum);
            if (child != NULL_NODE) {
                char *childPageStart;
                TRY(pfFileHandle.GetThisPageData(child, childPageStart));
                auto childTreeNode = (IX_BPlusTreeNode *) childPageStart;
                childTreeNode->father = prevChild;
                TRY(pfFileHandle.MarkDirty(child));
                TRY(pfFileHandle.UnpinPage(child));
            }
        }
        TRY(pfFileHandle.MarkDirty(prevChild));
        TRY(pfFileHandle.MarkDirty(father));
        TRY(pfFileHandle.MarkDirty(cur));
        TRY(pfFileHandle.UnpinPage(prevChild));
        TRY(pfFileHandle.UnpinPage(father));
        TRY(pfFileHandle.UnpinPage(cur));
        TRY(Delete(father, GetKeyAt(curPageStart, 0), GetValueAt(curPageStart, 0)));
        return OK_RC;
    }
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
            TRY(pfFileHandle.UnpinPage(deleteChild));
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
    TRY(pfFileHandle.MarkDirty(cur));
    TRY(pfFileHandle.UnpinPage(cur));
    if (deleteChild != NULL_NODE) {
        TRY(pfFileHandle.DisposePage(deleteChild))
    }
    if (curTreeNode->keyNum * 2 < ixFileHeader.maxKeyNum) {
        TRY(Redistribute(cur));
    }
    return OK_RC;
}

bool IX_IndexHandle::Compare(CompOp compOp, void *keyLeft, void *valueLeft, void *keyRight, void *valueRight) {
    RM_RID left = *(RM_RID *) valueLeft, right = *(RM_RID *) valueRight;
    int cmp = Utils::Cmp(keyLeft, keyRight, ixFileHeader.attrType, ixFileHeader.attrLength);
    switch (compOp) {
        case NO_OP:
            return true;
        case EQ_OP:
            return cmp == 0 && left == right;
        case LT_OP:
            if (cmp == 0)return left < right;
            else return cmp < 0;
        case LE_OP:
            if (cmp == 0)return left <= right;
            else return cmp <= 0;
        case NE_OP:
            if (cmp == 0) return left != right;
            else return true;
        case GT_OP:
            if (cmp == 0) return left > right;
            else return cmp > 0;
        case GE_OP:
            if (cmp == 0) return left >= right;
            else return cmp >= 0;
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
