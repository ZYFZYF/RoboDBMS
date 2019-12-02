//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBOSQL_PF_BUFFERMANAGER_H
#define ROBOSQL_PF_BUFFERMANAGER_H

#include "../Constant.h"
#include "PF_Constant.h"
#include "PF_HashTable.h"

#define INVALID_SLOT  (-1)

struct PF_BufPageDesc {
    char *pData;      // page contents
    int next;        // next in the linked list of buffer pages
    int prev;        // prev in the linked list of buffer pages
    int bDirty;      // TRUE if page is dirty
    short int pinCount;    // pin count
    PageNum pageNum;     // page number for this page
    int fd;          // OS file descriptor of this page
};

class PF_BufferManager {
public:

    PF_BufferManager(int numPages);             // Constructor - allocate
    // numPages buffer pages
    ~PF_BufferManager();                         // Destructor

    // Read pageNum into buffer, point *ppBuffer to location
    RC GetPage(int fd, PageNum pageNum, char **ppBuffer,
               int bMultiplePins = TRUE);

    // Allocate a new page in the buffer, point *ppBuffer to its location
    RC AllocatePage(int fd, PageNum pageNum, char **ppBuffer);

    RC MarkDirty(int fd, PageNum pageNum);  // Mark page dirty
    RC UnpinPage(int fd, PageNum pageNum);  // Unpin page from the buffer
    RC FlushPages(int fd);                   // Flush pages for file

    // Force a page to the disk, but do not remove from the buffer pool
    RC ForcePages(int fd, PageNum pageNum);


    // Remove all entries from the Buffer Manager.
    RC ClearBuffer();

    // Display all entries in the buffer
    RC PrintBuffer();

    // Attempts to resize the buffer to the new size
    RC ResizeBuffer(int iNewSize);

    // Three Methods for manipulating raw memory buffers.  These memory
    // locations are handled by the buffer manager, but are not
    // associated with a particular file.  These should be used if you
    // want memory that is bounded by the size of the buffer pool.

    // Return the size of the block that can be allocated.
    RC GetBlockSize(int &length) const;

    // Allocate a memory chunk that lives in buffer manager
    RC AllocateBlock(char *&buffer);

    // Dispose of a memory chunk managed by the buffer manager.
    RC DisposeBlock(char *buffer);

private:
    RC InsertFree(int slot);                 // Insert slot at head of free
    RC LinkHead(int slot);                 // Insert slot at head of used
    RC Unlink(int slot);                 // Unlink slot
    RC InternalAlloc(int &slot);                // Get a slot to use

    // Read a page
    RC ReadPage(int fd, PageNum pageNum, char *dest);

    // Write a page
    RC WritePage(int fd, PageNum pageNum, char *source);

    // Init the page desc entry
    RC InitPageDesc(int fd, PageNum pageNum, int slot);

    PF_BufPageDesc *bufTable;                     // info on buffer pages
    PF_HashTable hashTable;                     // Hash table object
    int numPages;                      // # of pages in the buffer
    int pageSize;                      // Size of pages in the buffer
    int first;                         // MRU page slot
    int last;                          // LRU page slot
    int free;                          // head of free list
};


#endif //ROBOSQL_PF_BUFFERMANAGER_H
