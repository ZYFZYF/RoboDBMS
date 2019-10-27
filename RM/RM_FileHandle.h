//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBODBMS_RM_FILEHANDLE_H
#define ROBODBMS_RM_FILEHANDLE_H


#include "../def.h"
#include "RM_Record.h"

class RM_FileHandle {
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
};


#endif //ROBODBMS_RM_FILEHANDLE_H
