//
// Created by 赵鋆峰 on 2019/10/27.
//

#include "PF_HashTable.h"
//
// File:        pf_hashtable.cc
// Description: PF_HashTable class implementation
// Authors:     Hugo Rivero (rivero@cs.stanford.edu)
//              Dallan Quass (quass@cs.stanford.edu)
//

//
// PF_HashTable
//
// Desc: Constructor for PF_HashTable object, which allows search, insert,
//       and delete of hash table entries.
// In:   numBuckets - number of hash table buckets
//
PF_HashTable::PF_HashTable(int _numBuckets) {
    // Initialize numBuckets local variable from parameter
    this->numBuckets = _numBuckets;

    // Allocate memory for hash table
    hashTable = new PF_HashEntry *[numBuckets];

    // Initialize all buckets to empty
    for (int i = 0; i < numBuckets; i++)
        hashTable[i] = NULL;
}

//
// ~PF_HashTable
//
// Desc: Destructor
//
PF_HashTable::~PF_HashTable() {
    // Clear out all buckets
    for (int i = 0; i < numBuckets; i++) {

        // Delete all entries in the bucket
        PF_HashEntry *entry = hashTable[i];
        while (entry != NULL) {
            PF_HashEntry *next = entry->next;
            delete entry;
            entry = next;
        }
    }

    // Finally delete the hash table
    delete[] hashTable;
}

//
// Find
//
// Desc: Find a hash table entry.
// In:   fd - file descriptor
//       pageNum - page number
// Out:  slot - set to slot associated with fd and pageNum
// Ret:  PF return code
//
RC PF_HashTable::Find(int fd, PageNum pageNum, int &slot) {
    // Get which bucket it should be in
    int bucket = Hash(fd, pageNum);

    if (bucket < 0)
        return (PF_HASHNOTFOUND);

    // Go through the linked list of this bucket
    for (PF_HashEntry *entry = hashTable[bucket];
         entry != NULL;
         entry = entry->next) {
        if (entry->fd == fd && entry->pageNum == pageNum) {

            // Found it
            slot = entry->slot;
            return OK_RC;
        }
    }

    // Didn't find it
    return (PF_HASHNOTFOUND);
}

//
// Insert
//
// Desc: Insert a hash table entry
// In:   fd - file descriptor
//       pagenum - page number
//       slot - slot associated with fd and pageNum
// Ret:  PF return code
//
RC PF_HashTable::Insert(int fd, PageNum pageNum, int slot) {
    // Get which bucket it should be in
    int bucket = Hash(fd, pageNum);

    // Check entry doesn't already exist in the bucket
    PF_HashEntry *entry;
    for (entry = hashTable[bucket];
         entry != NULL;
         entry = entry->next) {
        if (entry->fd == fd && entry->pageNum == pageNum)
            return (PF_HASHPAGEEXIST);
    }

    // Allocate memory for new hash entry
    if ((entry = new PF_HashEntry) == NULL)
        return (PF_NOMEM);

    // Insert entry at head of list for this bucket
    entry->fd = fd;
    entry->pageNum = pageNum;
    entry->slot = slot;
    entry->next = hashTable[bucket];
    entry->prev = NULL;
    if (hashTable[bucket] != NULL)
        hashTable[bucket]->prev = entry;
    hashTable[bucket] = entry;

    // Return ok
    return OK_RC;
}

//
// Delete
//
// Desc: Delete a hash table entry
// In:   fd - file descriptor
//       pagenum - page number
// Ret:  PF return code
//
RC PF_HashTable::Delete(int fd, PageNum pageNum) {
    // Get which bucket it should be in
    int bucket = Hash(fd, pageNum);

    // Find the entry is in this bucket
    PF_HashEntry *entry;
    for (entry = hashTable[bucket];
         entry != NULL;
         entry = entry->next) {
        if (entry->fd == fd && entry->pageNum == pageNum)
            break;
    }

    // Did we find hash entry?
    if (entry == NULL)
        return (PF_HASHNOTFOUND);

    // Remove this entry
    if (entry == hashTable[bucket])
        hashTable[bucket] = entry->next;
    if (entry->prev != NULL)
        entry->prev->next = entry->next;
    if (entry->next != NULL)
        entry->next->prev = entry->prev;
    delete entry;

    // Return ok
    return OK_RC;
}


