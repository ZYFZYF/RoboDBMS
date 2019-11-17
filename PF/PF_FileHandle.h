//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBOSQL_PF_FILEHANDLE_H
#define ROBOSQL_PF_FILEHANDLE_H


#include "../def.h"
#include "PF_PageHandle.h"
#include "PF_BufferManager.h"

//
// PF_FileHdr: Header structure for files
//
struct PF_FileHdr {
    int firstFree;     // first free page in the linked list
    int numPages;      // # of pages in the file
};

class PF_FileHandle {
    friend class PF_Manager;

public:
    PF_FileHandle();                            // Default constructor
    ~PF_FileHandle();                            // Destructor

    // Copy constructor
    PF_FileHandle(const PF_FileHandle &fileHandle);

    // Overload =
    PF_FileHandle &operator=(const PF_FileHandle &fileHandle);

    // Get the first page
    RC GetFirstPage(PF_PageHandle &pageHandle) const;

    // Get the next page after current
    RC GetNextPage(PageNum current, PF_PageHandle &pageHandle) const;

    // Get a specific page
    RC GetThisPage(PageNum pageNum, PF_PageHandle &pageHandle) const;

    // Get the last page
    RC GetLastPage(PF_PageHandle &pageHandle) const;

    // Get the prev page after current
    RC GetPrevPage(PageNum current, PF_PageHandle &pageHandle) const;

    RC AllocatePage(PF_PageHandle &pageHandle);    // Allocate a new page
    RC DisposePage(PageNum pageNum);              // Dispose of a page
    RC MarkDirty(PageNum pageNum) const;        // Mark page as dirty
    RC UnpinPage(PageNum pageNum) const;        // Unpin the page

    // Flush pages from buffer pool.  Will write dirty pages to disk.
    RC FlushPages() const;

    // Force a page or pages to disk (but do not remove from the buffer pool)
    RC ForcePages(PageNum pageNum = ALL_PAGES) const;

    RC GetThisPageData(PageNum pageNum, char *&pData);

private:

    // IsValidPageNum will return TRUE if page number is valid and FALSE
    // otherwise
    int IsValidPageNum(PageNum pageNum) const;

    PF_BufferManager *pBufferMgr;                      // pointer to buffer manager
    PF_FileHdr hdr;                                // file header
    int bFileOpen;                                 // file open flag
    int bHdrChanged;                               // dirty flag for file hdr
    int unixfd;                                    // OS file descriptor
};


#endif //ROBOSQL_PF_FILEHANDLE_H
