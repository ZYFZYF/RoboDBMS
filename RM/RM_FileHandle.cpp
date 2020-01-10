//
// Created by 赵鋆峰 on 2019/10/27.
//

#include <cmath>
#include <cstring>
#include <iostream>
#include "RM_FileHandle.h"
#include "../utils/PrintError.h"

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

RC RM_FileHandle::FindNextOne(MultiBits *bitmap, int size, int start, int &slot) const {
    for (int i = start; i < size; i++) {
        int index, bit;
        GetBitPosition(i, index, bit);
        //printf("i = %d, bit = %d\n", i, bitmap[index] >> bit & 1);
        if (bitmap[index] >> bit & 1) {
            slot = i;
            return OK_RC;
        }
    }
    return RM_ENDOFPAGE;
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
    isFileOpen = false;
    isHeaderModified = false;
}

RM_FileHandle::~RM_FileHandle() = default;

RC RM_FileHandle::GetRec(const RM_RID &rmRid, RM_Record &rec) const {
    RC rc;
    //先拿到RID的页号和槽位
    int page = -1, slot = -1;
    TRY(rmRid.GetPageNumAndSlotNum(page, slot));
    //拿到这一页的handle，成功之后退出时必须Unpin掉这一页
    PF_PageHandle pfPageHandle;
    TRY(pfFileHandle.GetThisPage(page, pfPageHandle));
    //拿到PageHeader以及Bitmap
    RM_PageHeader *rmPageHeader;
    MultiBits *bitmap;
    if ((rc = GetPageHeaderAndBitmap(pfPageHandle, rmPageHeader, bitmap))) {
        goto safe_exit;
    }
    //确保这个槽位上有这条记录，通过访问这一页的bitmap来实现
    bool exist;
    if ((rc = GetBit(bitmap, rmFileHeader.recordNumPerPage, slot, exist))) {
        goto safe_exit;
    }
    if (!exist) {
        rc = RM_INVALIDRECORD;
        goto safe_exit;
    }
    //取出record
    if ((rc = rec.Set(rmRid, bitmap + rmFileHeader.bitMapSize + slot * (rmFileHeader.recordSize),
                      rmFileHeader.recordSize))) {
        goto safe_exit;
    }
    safe_exit:
    TRY(pfFileHandle.UnpinPage(page));
    return rc;
}

RC RM_FileHandle::InsertRec(const char *pData, RM_RID &rmRid) {
    RC rc;
    PF_PageHandle pfPageHandle;
    PageNum pageNum;
    //先定位页，没页就分配新的
    if (rmFileHeader.firstFreePage == NO_MORE_FREE_PAGE) {
        AllocateNewPage(pfPageHandle, pageNum);
    } else {
        pageNum = rmFileHeader.firstFreePage;
        TRY(pfFileHandle.GetThisPage(pageNum, pfPageHandle));
    }
    //然后拿到页的信息
    char *bitmap;
    RM_PageHeader *rmPageHeader;
    int index;
    if ((rc = GetPageHeaderAndBitmap(pfPageHandle, rmPageHeader, bitmap))) {
        goto safe_exit;
    }
    //再从bitmap中找到free的第一条
    if ((rc = FindFirstZero(bitmap, rmFileHeader.recordNumPerPage, index))) {
        goto safe_exit;
    }
    //将其设置为1
    if ((rc = SetBit(bitmap, rmFileHeader.recordNumPerPage, index))) {
        goto safe_exit;
    }
    //把record内容拷贝过去
    memcpy(bitmap + rmFileHeader.bitMapSize + index * (rmFileHeader.recordSize), pData, rmFileHeader.recordSize);
    //更新page的header，如果满了就指向下一个，因为每次都从开头插所以能保证下一个freepage一定是真正free的
    if (++rmPageHeader->recordNum == rmFileHeader.recordNumPerPage) {
        rmFileHeader.firstFreePage = rmPageHeader->nextFreePage;
        isHeaderModified = true;
    }
    //设置rid的返回值
    rmRid = RM_RID(pageNum, index);
    safe_exit:
    TRY(pfFileHandle.MarkDirty(pageNum));
    TRY(pfFileHandle.UnpinPage(pageNum));
    return rc;
}

RC RM_FileHandle::DeleteRec(const RM_RID &rmRid) {
    RC rc;
    PF_PageHandle pfPageHandle;
    //先从RID中拿到page和slot
    PageNum pageNum;
    SlotNum slotNum;
    bool isOne;
    TRY(rmRid.GetPageNumAndSlotNum(pageNum, slotNum));
    //把页拿出来
    TRY(pfFileHandle.GetThisPage(pageNum, pfPageHandle));
    //然后拿到页的信息
    char *bitmap;
    RM_PageHeader *rmPageHeader;
    if ((rc = GetPageHeaderAndBitmap(pfPageHandle, rmPageHeader, bitmap))) {
        goto safe_exit;
    }
    //看看这个位置是否确实被使用
    if ((rc = GetBit(bitmap, rmFileHeader.recordNumPerPage, slotNum, isOne))) {
        goto safe_exit;
    }
    if (!isOne) {
        rc = RM_RIDDELETED;
        goto safe_exit;
    }
    //删除其占用标志
    if ((rc = ClearBit(bitmap, rmFileHeader.recordNumPerPage, slotNum))) {
        goto safe_exit;
    }
    //更新pageheader的信息和fileheader的信息
    if (rmPageHeader->recordNum-- == rmFileHeader.recordNumPerPage) {
        rmPageHeader->nextFreePage = rmFileHeader.firstFreePage;
        rmFileHeader.firstFreePage = pageNum;
        isHeaderModified = true;
    };

    safe_exit:
    TRY(pfFileHandle.MarkDirty(pageNum));
    TRY(pfFileHandle.UnpinPage(pageNum));
    return rc;
}

RC RM_FileHandle::UpdateRec(const RM_Record &rec) {
    RC rc;
    //定义变量
    bool exist;
    char *data;
    //先用rec拿到RID
    RM_RID rmRid;
    TRY(rec.GetRid(rmRid));
    //拿到RID的页号和槽位
    int page = -1, slot = -1;
    TRY(rmRid.GetPageNumAndSlotNum(page, slot));
    printf("Get page = %d and slot = %d\n", page, slot);
    //拿到这一页的handle，成功之后退出时必须Unpin掉这一页
    PF_PageHandle pfPageHandle;
    TRY(pfFileHandle.GetThisPage(page, pfPageHandle));
    //拿到PageHeader以及Bitmap
    RM_PageHeader *rmPageHeader;
    MultiBits *bitmap;
    char *recData;
    if ((rc = GetPageHeaderAndBitmap(pfPageHandle, rmPageHeader, bitmap))) {
        goto safe_exit;
    }
    //确保这个槽位上有这条记录，通过访问这一页的bitmap来实现
    if ((rc = GetBit(bitmap, rmFileHeader.recordNumPerPage, slot, exist))) {
        goto safe_exit;
    }
    if (!exist) {
        rc = RM_INVALIDRECORD;
        goto safe_exit;
    }
    //设置record
    data = bitmap + rmFileHeader.bitMapSize + slot * (rmFileHeader.recordSize);
    TRY(rec.GetData(recData));
    memcpy(data, recData, rmFileHeader.recordSize);
    safe_exit:
    TRY(pfFileHandle.MarkDirty(page));
    TRY(pfFileHandle.UnpinPage(page));
    return rc;
}

//TODO: 实现强制刷新
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

RC RM_FileHandle::GetPageHeaderAndBitmap(PF_PageHandle &pph, RM_PageHeader *&rmPageHeader, MultiBits *&bitmap) const {
    char *data;
    TRY(pph.GetData(data));
    rmPageHeader = (RM_PageHeader *) data;
    bitmap = (MultiBits *) (data + rmFileHeader.bitMapOffset);
    return OK_RC;
}

RC RM_FileHandle::AllocateNewPage(PF_PageHandle &pfPageHandle, PageNum &pageNum) {
    //申请新页
    TRY(pfFileHandle.AllocatePage(pfPageHandle));
    TRY(pfPageHandle.GetPageNum(pageNum));
    //拿到新页的header和bitmap
    char *bitmap;
    RM_PageHeader *rmPageHeader;
    TRY(GetPageHeaderAndBitmap(pfPageHandle, rmPageHeader, bitmap));
    //初始化header，将freepage链表串起来
    rmPageHeader->recordNum = 0;
    rmPageHeader->nextFreePage = rmFileHeader.firstFreePage;
    rmFileHeader.firstFreePage = pageNum;
    ResetBitmap(bitmap, rmFileHeader.recordNumPerPage);
    rmFileHeader.pageCount++;
    isHeaderModified = true;
    //这里不应该把header变成脏的么 updated:已经添加
    return OK_RC;
}

RC RM_FileHandle::GetNextRecord(PageNum page, SlotNum slot, RM_Record &rec, PF_PageHandle &pfPageHandle,
                                bool findInNextPage) {
    char *bitmap;
    struct RM_PageHeader *rmPageHeader;
    int nextRec;
    PageNum nextRecPage = page;
    SlotNum nextRecSlot;

    //拿下一页的第一条
    if (findInNextPage) {
        while (true) {
            //取出下一页并且拿到page以及页头
            if ((pfFileHandle.GetNextPage(nextRecPage, pfPageHandle)) == PF_EOF) {
                return RM_EOF;
            }
            TRY(pfPageHandle.GetPageNum(nextRecPage));
            TRY(GetPageHeaderAndBitmap(pfPageHandle, rmPageHeader, bitmap));
            //如果找到一个1那么就可以返回了，否则继续下一页
            if (FindNextOne(bitmap, rmFileHeader.recordNumPerPage, 0, nextRec) != RM_ENDOFPAGE)
                break;
            TRY(pfFileHandle.UnpinPage(nextRecPage));
        }
    } else {
        // 直接用bitmap拿到下一个1的位置
        TRY(GetPageHeaderAndBitmap(pfPageHandle, rmPageHeader, bitmap));
        if (FindNextOne(bitmap, rmFileHeader.recordNumPerPage, slot + 1, nextRec) == RM_ENDOFPAGE) {
            return RM_EOF;
        }
    }

    nextRecSlot = nextRec;
    RM_RID rid(nextRecPage, nextRecSlot);
    //printf("nextRecPage = %d, nextRecSlot = %d\n", nextRecPage, nextRecSlot);
    TRY(rec.Set(rid, bitmap + (rmFileHeader.bitMapSize) + (nextRecSlot) * (rmFileHeader.recordSize),
                rmFileHeader.recordSize));
    return OK_RC;
}

int RM_FileHandle::GetRecordCount() {
    PF_PageHandle pfPageHandle;
    int count = 0;
    PageNum pageNum = 0;
    while (pfFileHandle.GetNextPage(pageNum, pfPageHandle) != PF_EOF) {
        pfPageHandle.GetPageNum(pageNum);
        char *bitmap;
        struct RM_PageHeader *rph;
        DO(GetPageHeaderAndBitmap(pfPageHandle, rph, bitmap))
        count += rph->recordNum;
        DO(pfFileHandle.UnpinPage(pageNum))
    }
    return count;
}
