//
// Created by 赵鋆峰 on 2019/10/27.
//

#include <cmath>
#include <cstring>
#include <iostream>
#include "RM_FileHandle.h"

const int BitsNum = sizeof(MultiBits) * 8;

RC RM_FileHandle::ResetBitmap(MultiBits *bitmap, int size) {
    for (int i = 0; i < ConvertBitToMultiBits(size); i++) {
        bitmap[i] = 0;
    }
    return PF_NOBUF;
}

RC RM_FileHandle::SetBit(MultiBits *bitmap, int size, int slot) {
    if (slot < 0 || slot >= size) {
        return RM_INVALIDBITIND;
    }
    int index, bit;
    GetBitPosition(slot, index, bit);
    if (bitmap[index] >> bit & 1) {
        return RM_BITINUSE;
    }
    bitmap[index] |= 1 << bit;
    return OK_RC;
}

RC RM_FileHandle::ClearBit(MultiBits *bitmap, int size, int slot) {
    if (slot < 0 || slot >= size) {
        return RM_INVALIDBITIND;
    }
    int index, bit;
    GetBitPosition(slot, index, bit);
    if (!(bitmap[index] >> bit & 1)) {
        return RM_BITNOTINUSE;
    }
    //std::cerr << index << ' ' << bit << ' ' << int(bitmap[index]) << ' ';
    bitmap[index] &= ~(1 << bit);
    //std::cerr << int(bitmap[index]) << std::endl;
    return OK_RC;
}

RC RM_FileHandle::GetBit(MultiBits *bitmap, int size, int slot, bool &inUse) const {
    if (slot < 0 || slot >= size) {
        return RM_INVALIDBITIND;
    }
    int index, bit;
    GetBitPosition(slot, index, bit);
    inUse = bitmap[index] >> bit & 1;
    return OK_RC;
}

RC RM_FileHandle::FindFirstZero(MultiBits *bitmap, int size, int &slot) const {
    for (int i = 0; i < size; i++) {
        //std::cerr << "i get " << ' ' << i << std::endl;
        int index, bit;
        GetBitPosition(i, index, bit);
        if (!(bitmap[index] >> bit & 1)) {
            slot = i;
            return OK_RC;
        }
    }
    return RM_BITMAPISFULL;
}

int RM_FileHandle::ConvertBitToMultiBits(int size) {
    return (size - 1) / BitsNum + 1;
}

RC RM_FileHandle::GetBitPosition(int slot, int &index, int &bit) const {
    index = slot / BitsNum;
    bit = slot - index * BitsNum;
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
    if ((rc = GetBit(bitmap, rfh.recordNumPerPage, slot, exist))) {
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
    RC rc = OK_RC;
    PF_PageHandle pph;
    PageNum pageNum;
    //先定位页，没页就分配新的
    if (rfh.firstFreePage == NO_MORE_FREE_PAGE) {
        AllocateNewPage(pph, pageNum);
    } else {
        pageNum = rfh.firstFreePage;
        if ((rc = pfh.GetThisPage(pageNum, pph))) {
            return rc;
        }
    }
    //然后拿到页的信息
    char *bitmap;
    RM_PageHeader *rph;
    int index;
    if ((rc = GetPageHeaderAndBitmap(pph, rph, bitmap))) {
        goto safe_exit;
    }
    //再从bitmap中找到free的第一条
    if ((rc = FindFirstZero(bitmap, rfh.recordNumPerPage, index))) {
        goto safe_exit;
    }
    //将其设置为1
    if ((rc = SetBit(bitmap, rfh.recordNumPerPage, index))) {
        goto safe_exit;
    }
    //把record内容拷贝过去
    memcpy(bitmap + rfh.bitMapSize + index * (rfh.recordSize), pData, rfh.recordSize);
    //更新page的header，如果满了就指向下一个，因为每次都从开头插所以能保证下一个freepage一定是真正free的
    if (++rph->recordNum == rfh.recordNumPerPage) {
        rfh.firstFreePage = rph->nextFreePage;
    }
    //设置rid的返回值
    rmRid = RM_RID(pageNum, index);
    safe_exit:
    RC rc1;
    if ((rc1 = pfh.MarkDirty(pageNum)) || (rc1 = pfh.UnpinPage(pageNum))) {
        return rc1;
    }
    return rc;
}

RC RM_FileHandle::DeleteRec(const RM_RID &rmRid) {
    RC rc = OK_RC;
    PF_PageHandle pph;
    //先从RID中拿到page和slot
    PageNum pageNum;
    SlotNum slotNum;
    bool isOne;
    if ((rc = rmRid.GetPageNumAndSlotNum(pageNum, slotNum))) {
        return rc;
    }
    //把页拿出来
    if ((rc = pfh.GetThisPage(pageNum, pph))) {
        return rc;
    }
    //然后拿到页的信息
    char *bitmap;
    RM_PageHeader *rph;
    int index;
    if ((rc = GetPageHeaderAndBitmap(pph, rph, bitmap))) {
        goto safe_exit;
    }
    //看看这个位置是否确实被使用
    if ((rc = GetBit(bitmap, rfh.recordNumPerPage, slotNum, isOne))) {
        return rc;
    }
    if (!isOne) {
        rc = RM_RIDDELETED;
        goto safe_exit;
    }
    //删除其占用标志
    if ((rc = ClearBit(bitmap, rfh.recordNumPerPage, slotNum))) {
        return rc;
    }
    if (rph->recordNum-- == rfh.recordNumPerPage) {
        rph->nextFreePage = rfh.firstFreePage;
        rfh.firstFreePage = pageNum;
    };

    safe_exit:
    RC rc1;
    if ((rc1 = pfh.MarkDirty(pageNum)) || (rc1 = pfh.UnpinPage(pageNum))) {
        return rc1;
    }
    return rc;
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

    if ((rc = GetBit(bitmap, rfh.recordNumPerPage, slot, exist))) {
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

RC RM_FileHandle::AllocateNewPage(PF_PageHandle &pph, PageNum &pageNum) {
    RC rc;
    //申请新页
    if ((rc = pfh.AllocatePage(pph))) {
        return rc;
    }
    if ((rc = pph.GetPageNum(pageNum))) {
        return rc;
    }
    //拿到新页的header和bitmap
    char *bitmap;
    RM_PageHeader *rph;
    if ((rc = GetPageHeaderAndBitmap(pph, rph, bitmap))) {
        return rc;
    }
    //初始化header，将freepage链表串起来
    rph->recordNum = 0;
    rph->nextFreePage = rfh.firstFreePage;
    rfh.firstFreePage = pageNum;
    ResetBitmap(bitmap, rfh.recordNumPerPage);
    rfh.pageCount++;
    //这里不应该把header变成脏的么
    return OK_RC;
}
