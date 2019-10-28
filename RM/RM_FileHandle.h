//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBODBMS_RM_FILEHANDLE_H
#define ROBODBMS_RM_FILEHANDLE_H


#include "../def.h"
#include "RM_Record.h"
#include "../PF/PF_FileHandle.h"
#include "def.h"

class RM_FileHandle {
    friend class RM_Manager;

public:
    RM_FileHandle();

    ~RM_FileHandle();

    // Given a RM_RID, return the record
    RC GetRec(const RM_RID &rmRid, RM_Record &rec) const;

    RC InsertRec(const char *pData, RM_RID &rmRid);       // Insert a new record

    RC DeleteRec(const RM_RID &rmRid);                    // Delete a record

    RC UpdateRec(const RM_Record &rec);              // Update a record

    // Forces a page (along with any contents stored in this class)
    // from the buffer pool to disk.  Default value forces all pages.
    RC ForcePages(PageNum pageNum = ALL_PAGES);

//private:

    RC GetNextRecord(PageNum page, SlotNum slot, RM_Record &rec, PF_PageHandle &pph, bool findInNextPage);

    RC AllocateNewPage(PF_PageHandle &pph, PageNum &pageNum);

    RC GetPageHeaderAndBitmap(PF_PageHandle &pph, RM_PageHeader *&rph, MultiBits *&bitmap) const;

    RC GetBitPosition(int slot, int &index, int &bit) const;

    RC ResetBitmap(MultiBits *bitmap, int size);

    RC SetBit(MultiBits *bitmap, int size, int slot);

    RC ClearBit(MultiBits *bitmap, int size, int slot);

    RC GetBit(MultiBits *bitmap, int size, int slot, bool &inUse) const;

    //TODO：优化这两个函数的实现，这个复杂度太高了，又要能方便地序列化和反序列化
    RC FindFirstZero(MultiBits *bitmap, int size, int &slot) const;

    RC FindNextOne(MultiBits *bitmap, int size, int start, int &slot) const;

    static int ConvertBitToMultiBits(int size);

    static int CalcRecordNumPerPage(int recordSize);

    PF_FileHandle pfh;

    RM_FileHeader rfh;

    bool isFileOpen;

    bool isHeaderModified;

};


#endif //ROBODBMS_RM_FILEHANDLE_H
