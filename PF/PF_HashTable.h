//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBOSQL_PF_HASHTABLE_H
#define ROBOSQL_PF_HASHTABLE_H

#include "def.h"
#include "../def.h"

//
// HashEntry - Hash table bucket entries
//
struct PF_HashEntry {
    PF_HashEntry *next;   // next hash table element or NULL
    PF_HashEntry *prev;   // prev hash table element or NULL
    int fd;      // file descriptor
    PageNum pageNum; // page number
    int slot;    // slot of this page in the buffer
};

//
// PF_HashTable - allow search, insertion, and deletion of hash table entries
//
class PF_HashTable {
public:
    PF_HashTable(int numBuckets);           // Constructor
    ~PF_HashTable();                         // Destructor
    RC Find(int fd, PageNum pageNum, int &slot);

    // Set slot to the hash table
    // entry for fd and pageNum
    RC Insert(int fd, PageNum pageNum, int slot);

    // Insert a hash table entry
    RC Delete(int fd, PageNum pageNum);  // Delete a hash table entry

private:
    int Hash(int fd, PageNum pageNum) const {
        return ((fd + 1) * (pageNum + 1) % numBuckets + numBuckets) % numBuckets;
        //return ((fd + pageNum) % numBuckets);
    }   // Hash function
    int numBuckets;                               // Number of hash table buckets
    PF_HashEntry **hashTable;                     // Hash table
};


#endif //ROBOSQL_PF_HASHTABLE_H
