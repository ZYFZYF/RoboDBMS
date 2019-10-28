//
// Created by 赵鋆峰 on 2019/10/27.
//

#include <cmath>
#include <cstring>
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

RC RM_FileHandle::GetBit(MultiBits *bitmap, int size, int index, bool &inUse) const {
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

RC RM_FileHandle::GetBitPosition(int index, int &slot, int &bit) const {
    slot = index / BitsNum, bit = index - slot * BitsNum;
    return OK_RC;
}

RM_FileHandle::RM_FileHandle() {

}

RM_FileHandle::~RM_FileHandle() {

}

RC RM_FileHandle::GetRec(const RM_RID &rmRid, RM_Record &rec) const {
    RC rc = OK_RC;
    //先拿到RID的页号和槽位
    int page = -1, slot = -1;
    if ((rc = rmRid.GetPageNumAndSlotNum(page, slot))) {
        return rc;
    }
    //拿到这一页的handle，成功之后退出时必须Unpin掉这一页
    PF_PageHandle pph;
    if ((rc = pfh.GetThisPage(page, pph))) {
        return rc;
    }
    //拿到PageHeader以及Bitmap
    RM_PageHeader *rph;
    MultiBits *bitmap;
    if ((rc = GetPageHeaderAndBitmap(pph, rph, bitmap))) {
        goto safe_exit;
    }
    //确保这个槽位上有这条记录，通过访问这一页的bitmap来实现
    bool exist;
    if ((rc = GetBit(bitmap, rfh.bitMapSize, slot, exist))) {
        goto safe_exit;
    }
    if (!exist) {
        rc = RM_INVALIDRECORD;
        goto safe_exit;
    }
    //取出record
    if ((rc = rec.Set(rmRid, bitmap + rfh.bitMapSize + slot * (rfh.recordSize), rfh.recordSize))) {
        goto safe_exit;
    }
    safe_exit:
    RC rc1;
    if ((rc1 = pfh.UnpinPage(page))) {
        return rc1;
    };
    return rc;
}

RC RM_FileHandle::InsertRec(const char *pData, RM_RID &rmRid) {
    return PF_NOBUF;
}

RC RM_FileHandle::DeleteRec(const RM_RID &rmRid) {
    return PF_NOBUF;
}

RC RM_FileHandle::UpdateRec(const RM_Record &rec) {
    RC rc = OK_RC;
    //定义变量
    bool exist;
    char *data;
    //先用rec拿到RID
    RM_RID rmRid;
    if ((rc = rec.GetRid(rmRid))) {
        return rc;
    }
    //拿到RID的页号和槽位
    int page = -1, slot = -1;
    if ((rc = rmRid.GetPageNumAndSlotNum(page, slot))) {
        return rc;
    }
    //拿到这一页的handle，成功之后退出时必须Unpin掉这一页
    PF_PageHandle pph;
    if ((rc = pfh.GetThisPage(page, pph))) {
        return rc;
    }
    //拿到PageHeader以及Bitmap
    RM_PageHeader *rph;
    MultiBits *bitmap;
    if ((rc = GetPageHeaderAndBitmap(pph, rph, bitmap))) {
        goto safe_exit;
    }
    //确保这个槽位上有这条记录，通过访问这一页的bitmap来实现

    if ((rc = GetBit(bitmap, rfh.bitMapSize, slot, exist))) {
        goto safe_exit;
    }
    if (!exist) {
        rc = RM_INVALIDRECORD;
        goto safe_exit;
    }
    //设置record
    data = bitmap + rfh.bitMapSize + slot * (rfh.recordSize);
    memcpy(data, &rec, rfh.recordSize);
    safe_exit:
    RC rc1;
    if ((rc1 = pfh.MarkDirty(page)) || (rc1 = pfh.UnpinPage(page))) {
        return rc1;
    }
    return rc;
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

RC RM_FileHandle::GetPageHeaderAndBitmap(PF_PageHandle &pph, RM_PageHeader *&rph, MultiBits *&bitmap) const {
    RC rc;
    char *data;
    if ((rc = pph.GetData(data))) {
        return rc;
    }
    rph = (RM_PageHeader *) data;
    bitmap = (MultiBits *) (data + rfh.bitMapOffset);
    return OK_RC;
}
