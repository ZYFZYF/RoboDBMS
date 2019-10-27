//
// Created by 赵鋆峰 on 2019/10/27.
//

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
