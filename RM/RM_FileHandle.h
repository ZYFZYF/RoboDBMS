//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBODBMS_RM_FILEHANDLE_H
#define ROBODBMS_RM_FILEHANDLE_H


#include "../def.h"
#include "RM_Record.h"
#include "../PF/PF_FileHandle.h"
#include "def.h"

struct RM_FileHeader {
    int recordSize;
    int recordNumPerPage;
    int bitMapOffset;
    int bitMapSize;
};

class RM_FileHandle {
    friend class RM_Manager;

public:
    RM_FileHandle();

    ~RM_FileHandle();

    // Given a RM_RID, return the record
    RC GetRec(const RM_RID &RM_RID, RM_Record &rec) const;

    RC InsertRec(const char *pData, RM_RID &RM_RID);       // Insert a new record

    RC DeleteRec(const RM_RID &RM_RID);                    // Delete a record
    RC UpdateRec(const RM_Record &rec);              // Update a record

    // Forces a page (along with any contents stored in this class)
    // from the buffer pool to disk.  Default value forces all pages.
    RC ForcePages(PageNum pageNum = ALL_PAGES);

private:
    RC GetBitPosition(int index, int &slot, int &bit);

    RC ResetBitmap(MultiBits *bitmap, int size);

    RC SetBit(MultiBits *bitmap, int size, int index);

    RC ClearBit(MultiBits *bitmap, int size, int index);

    RC GetBit(MultiBits *bitmap, int size, int index, bool &inUse);

    static int ConvertBitToMultiBits(int size);

    static int CalcRecordNumPerPage(int recordSize);

    PF_FileHandle pfh;

    RM_FileHeader rfh;

};


#endif //ROBODBMS_RM_FILEHANDLE_H
