//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBOSQL_PF_MANAGER_H
#define ROBOSQL_PF_MANAGER_H


#include "../def.h"
#include "PF_FileHandle.h"
#include "PF_BufferManager.h"

class PF_Manager {
public:
    PF_Manager();                              // Constructor
    ~PF_Manager();                              // Destructor
    RC CreateFile(const char *fileName);       // Create a new file
    RC DestroyFile(const char *fileName);       // Delete a file

    // Open and close file methods
    RC OpenFile(const char *fileName, PF_FileHandle &fileHandle);

    RC CloseFile(PF_FileHandle &fileHandle);

    // Three methods that manipulate the buffer manager.  The calls are
    // forwarded to the PF_BufferMgr instance and are called by parse.y
    // when the user types in a system command.
    RC ClearBuffer();

    RC PrintBuffer();

    RC ResizeBuffer(int iNewSize);

    // Three Methods for manipulating raw memory buffers.  These memory
    // locations are handled by the buffer manager, but are not
    // associated with a particular file.  These should be used if you
    // want to create structures in memory that are bounded by the size
    // of the buffer pool.
    //
    // Note: If you lose the pointer to the buffer that is passed back
    // from AllocateBlock or do not call DisposeBlock with that pointer
    // then the internal memory will always remain "pinned" in the buffer
    // pool and you will have lost a slot in the buffer pool.

    // Return the size of the block that can be allocated.
    RC GetBlockSize(int &length) const;

    // Allocate a memory chunk that lives in buffer manager
    RC AllocateBlock(char *&buffer);

    // Dispose of a memory chunk managed by the buffer manager.
    RC DisposeBlock(char *buffer);

private:
    PF_BufferManager *pBufferManager;                      // page-buffer manager
};


#endif //ROBOSQL_PF_MANAGER_H
