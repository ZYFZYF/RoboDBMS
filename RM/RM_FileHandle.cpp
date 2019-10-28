//
// Created by 赵鋆峰 on 2019/10/27.
//

#include <cmath>
#include "RM_FileHandle.h"

const int BitsNum = sizeof(MultiBits);

RC RM_FileHandle::ResetBitmap(MultiBits *bitmap, int size) {
    for (int i = 0; i < ConvertBitToMultiBits(size); i++) {
        bitmap[i] = 0;
    }
    return PF_NOBUF;
}

RC RM_FileHandle::SetBit(MultiBits *bitmap, int size, int index) {
    if (index < 0 || index >= size) {
        return RM_INVALIDBITIND;
    }
    int slot, bit;
    GetBitPosition(index, slot, bit);
    if (bitmap[slot] >> index & 1) {
        return RM_BITINUSE;
    }
    bitmap[slot] |= 1 << index;
    return OK_RC;
}

RC RM_FileHandle::ClearBit(MultiBits *bitmap, int size, int index) {
    if (index < 0 || index >= size) {
        return RM_INVALIDBITIND;
    }
    int slot, bit;
    GetBitPosition(index, slot, bit);
    if (!(bitmap[slot] >> index & 1)) {
        return RM_BITNOTINUSE;
    }
    bitmap[slot] &= !(1 << index);
}

RC RM_FileHandle::GetBit(MultiBits *bitmap, int size, int index, bool &inUse) {
    if (index < 0 || index >= size) {
        return RM_INVALIDBITIND;
    }
    int slot, bit;
    GetBitPosition(index, slot, bit);
    inUse = bitmap[slot] >> index & 1;
    return OK_RC;
}

int RM_FileHandle::ConvertBitToMultiBits(int size) {
    return (size - 1) / BitsNum + 1;
}

RC RM_FileHandle::GetBitPosition(int index, int &slot, int &bit) {
    slot = index / BitsNum, bit = index - slot * BitsNum;
    return OK_RC;
}

RM_FileHandle::RM_FileHandle() {

}

RM_FileHandle::~RM_FileHandle() {

}

RC RM_FileHandle::GetRec(const RM_RID &RM_RID, RM_Record &rec) const {

    return PF_NOBUF;
}

RC RM_FileHandle::InsertRec(const char *pData, RM_RID &RM_RID) {
    return PF_NOBUF;
}

RC RM_FileHandle::DeleteRec(const RM_RID &RM_RID) {
    return PF_NOBUF;
}

RC RM_FileHandle::UpdateRec(const RM_Record &rec) {
    return PF_NOBUF;
}

RC RM_FileHandle::ForcePages(PageNum pageNum) {
    return PF_NOBUF;
}

int RM_FileHandle::CalcRecordNumPerPage(int recordSize) {
    //复杂度高一点也没关系，只有新建表的时候会调用这个函数
    int totSpace = PF_PAGE_SIZE - RM_PAGE_HEADER_SIZE;
    int maxNum = totSpace / recordSize;
    int nowNeedSpace = maxNum * recordSize + maxNum;
    int oneSlotSize = sizeof(MultiBits) * 8;
    while (nowNeedSpace > totSpace) {
        maxNum--;
        nowNeedSpace -= recordSize;
        if (maxNum % oneSlotSize == 0) {
            nowNeedSpace -= oneSlotSize;
        }
    }
    return maxNum;
    //这样的算法有问题，因为会少算一部分
    //return floor((PF_PAGE_SIZE - RM_PAGE_HEADER_SIZE) / (recordSize + 1.0 / (8 * sizeof(MultiBits))));
}
