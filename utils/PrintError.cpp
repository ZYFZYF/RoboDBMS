//
// Created by 赵鋆峰 on 2019/10/27.
//
#include "../Constant.h"
#include <map>
#include <string>
#include <iostream>
#include "PrintError.h"

using namespace std;

map<RC, string> msg = {
        {OK_RC, "OK RETURN CODE"},

        {PF_PAGEPINNED, "PF WARNING: page pinned in buffer"},
        {PF_PAGENOTINBUF, "PF WARNING: page is not in the buffer"},
        {PF_INVALIDPAGE, "PF WARNING: invalid page number"},
        {PF_FILEOPEN, "PF WARNING: file open"},
        {PF_CLOSEDFILE, "PF WARNING: invalid file descriptor (file closed)"},
        {PF_PAGEFREE, "PF WARNING: page already free"},
        {PF_PAGEUNPINNED, "PF WARNING: page already unpinned"},
        {PF_EOF, "PF WARNING: end of file"},
        {PF_TOOSMALL, "PF WARNING: attempting to resize the buffer too small"},

        {PF_NOMEM, "PF ERROR: no memory"},
        {PF_NOBUF, "PF ERROR: no buffer space"},
        {PF_INCOMPLETEREAD, "PF ERROR: incomplete read of page from file"},
        {PF_INCOMPLETEWRITE, "PF ERROR: incomplete write of page to file"},
        {PF_HDRREAD, "PF ERROR: incomplete read of header from file"},
        {PF_HDRWRITE, "PF ERROR: incomplete write of header from file"},
        {PF_PAGEINBUF, "PF ERROR: new page to be allocated already in buffer"},
        {PF_HASHNOTFOUND, "PF ERROR: hash table entry not found"},
        {PF_HASHPAGEEXIST, "PF ERROR: page already in hash table"},
        {PF_INVALIDNAME, "PF ERROR: invalid file name"},

        {RM_EOF, "RM WARNING: file is not opened"},
        {RM_SLOTNOTINRANGE, "RM WARNING: SlotNum is out of range"},
        {RM_DELETEDRECORD, "RM WARNING: record is already deleted"},
        {RM_NOMORERECODE, "RM WARNING: no more records in scan"},
        {RM_NOTPROPERRECORD, "RM WARNING: Record is not properly initialized"},
        {RM_NOTPROPERRID, "RM WARNING: RID is not properly initialized"},
        {RM_SCANNOTOPEN, "RM WARNING: scan is not opened"},
        {RM_SCANNOTCLOSE, "RM WARNING: last opened scan is not closed"},
        {RM_INVALIDRECORD, "RM WARNING: invalid record page"},
        {x:RM_INVALIDBITIND, "RM WARNING: bit int is out of range"},
        {RM_BITINUSE, "RM WARNING: bit already in use"},
        {RM_BITNOTINUSE, "RM WARNING: bit not in use"},
        {RM_INVALIDRID, "RM WARNING: invalid record id"},
        {RM_NULLFILENAME, "RM WARNING: null file name"},
        {RM_BADRECORDSIZE, "RM WARNING: bad record size"},
        {RM_NOMEMORYLEFT, "RM WARNING: no enough memory to alloc"},
        {RM_BITMAPISFULL, "RM WARNING: bitmap is full"},
        {RM_RIDDELETED, "RM WARNING: rid already deleted"},
        {RM_INVALIDFILHANDLE, "RM WARNING: invalid file handle"},
        {RM_INVALIDSCAN, "RM WARNING: invalid file scan"},
        {RM_ENDOFPAGE, "RM WARNING: already end of page"},

        {RM_TOOLARGERECSIZE, "RM ERROR: recordSize is too large for current pagefile system"},

        {SP_NULLFILENAME, "SP WARNING: null file name"},
        {SP_UNIX, "SP ERROR: linux file operation error"},

        {IX_EOF, "IX WARNING: end of index, no more entry"},
        {IX_ALREADY_IN_BTREE, "IX WARNING: entry already in b+ tree"},
        {IX_INDEX_SCAN_CLOSE, "IX WARNING: index scan is closed cannot get next entry"},
        {IX_ALREADY_NOT_IN_BTREE, "IX WARNING: entry already not in b+ tree"},
        {IX_FIND_NON_EXIST, "IX ERROR: found non-existent entry"},
        {IX_NOT_FIND, "IX ERROR: not found entry"},
        {IX_INSERT_TWICE, "IX ERROR: insert one entry twice"},
        {IX_DELETE_TWICE, "IX ERROR: delete one entry twice"},
        {IX_DELETE_NON_EXIST, "IX ERROR: delete non exist entry"},

        {STAT_INVALID_ARGS, "STAT ERROR: invalid args"},
        {STAT_UNKNOWN_KEY, "STAT ERROR: unknown key"},

        {SM_INVALID_NAME, "SM WARNING: INVALID NAME"},
        {SM_TABLE_ALREADY_IN, "SM WARNING: TABLE ALREADY IN DATABASER"},
        {SM_TABLE_IS_FULL, "SM WARNING: TABLE IS FULL"},
        {SM_COLUMN_ALREADY_IN, "SM WARNING: COLUMN ALREADY IN TABLE"},
        {SM_COLUMN_IS_FULL, "SM WARNING: COLUMN IS FULL"},
        {SM_DB_ALREADY_IN, "SM WARNING: DATABASE ALREADY IN DBMS"},
        {SM_DB_NOT_EXIST, "SM WARNING: DATABASE NOT EXIST"},
        {SM_DB_IS_FULL, "SM WARNING: DATRABASE IS FULL"},
        {SM_NOT_USING_DATABASE, "SM WARNING: NOT USING DATABASE"},
        {SM_TABLE_NOT_EXIST, "SM WARNING: TABLE NOT EXIST IN DATABASE"},
        {SM_ALREADY_HAVE_PRIMARY_KEY, "SM WARNING: ALREADY HAVE PRIMARY KEY"},
        {SM_COLUMN_NOT_EXIST, "SM WARNING: COLUMN NOT EXIST IN TABLE"},
        {SM_FOREIGN_KEYS_AND_PRIMARY_KEYS_NOT_MATCH,
         "SM WARNING: THE NUMBER OF FOREIGN KEYS AND THE NUMBER OF PRIMARY KEYS ARE NOT MATCH"},
        {SM_PRIMARY_KEY_NOT_EXIST, "SM WARNING: PRIMARY KEY NOT EXIST"},
        {SM_FOREIGN_KEYS_IS_FULL, "SM WARNING: FOREIGN KEYS IS FULL"},
        {SM_REFERENCE_IS_FULL, "SM WARNING: REFERENCE IS FULL"},
        {SM_FOREIGN_KEY_NOT_EXIST, "SM WARNING: FOREIGN KEY NOT EXIST"},

        {SM_UNIX, " SM ERROR: UNIX ERROR"},


        {UNKNOWN_RC, "Sorry: we haven't name this warning or error"}
};

void printError(RC rc) {
    if (msg.find(rc) == msg.end()) {
        cerr << "Sorry. Rc is not recorded in note." << endl;
    } else {
        cerr << msg[rc] << endl;
    }
}