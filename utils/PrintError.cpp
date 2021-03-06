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
        {OK_RC,                                             "OK RETURN CODE"},

        {PF_PAGEPINNED,                                     "PF WARNING: page pinned in buffer"},
        {PF_PAGENOTINBUF,                                   "PF WARNING: page is not in the buffer"},
        {PF_INVALIDPAGE,                                    "PF WARNING: invalid page number"},
        {PF_FILEOPEN,                                       "PF WARNING: file open"},
        {PF_CLOSEDFILE,                                     "PF WARNING: invalid file descriptor (file closed)"},
        {PF_PAGEFREE,                                       "PF WARNING: page already free"},
        {PF_PAGEUNPINNED,                                   "PF WARNING: page already unpinned"},
        {PF_EOF,                                            "PF WARNING: end of file"},
        {PF_TOOSMALL,                                       "PF WARNING: attempting to resize the buffer too small"},

        {PF_NOMEM,                                          "PF ERROR: no memory"},
        {PF_NOBUF,                                          "PF ERROR: no buffer space"},
        {PF_INCOMPLETEREAD,                                 "PF ERROR: incomplete read of page from file"},
        {PF_INCOMPLETEWRITE,                                "PF ERROR: incomplete write of page to file"},
        {PF_HDRREAD,                                        "PF ERROR: incomplete read of header from file"},
        {PF_HDRWRITE,                                       "PF ERROR: incomplete write of header from file"},
        {PF_PAGEINBUF,                                      "PF ERROR: new page to be allocated already in buffer"},
        {PF_HASHNOTFOUND,                                   "PF ERROR: hash table entry not found"},
        {PF_HASHPAGEEXIST,                                  "PF ERROR: page already in hash table"},
        {PF_INVALIDNAME,                                    "PF ERROR: invalid file name"},

        {RM_EOF,                                            "RM WARNING: file is not opened"},
        {RM_SLOTNOTINRANGE,                                 "RM WARNING: SlotNum is out of range"},
        {RM_DELETEDRECORD,                                  "RM WARNING: record is already deleted"},
        {RM_NOMORERECODE,                                   "RM WARNING: no more records in scan"},
        {RM_NOTPROPERRECORD,                                "RM WARNING: Record is not properly initialized"},
        {RM_NOTPROPERRID,                                   "RM WARNING: RID is not properly initialized"},
        {RM_SCANNOTOPEN,                                    "RM WARNING: scan is not opened"},
        {RM_SCANNOTCLOSE,                                   "RM WARNING: last opened scan is not closed"},
        {RM_INVALIDRECORD,                                  "RM WARNING: invalid record page"},
        {RM_INVALIDBITIND,                                  "RM WARNING: bit int is out of range"},
        {RM_BITINUSE,                                       "RM WARNING: bit already in use"},
        {RM_BITNOTINUSE,                                    "RM WARNING: bit not in use"},
        {RM_INVALIDRID,                                     "RM WARNING: invalid record id"},
        {RM_NULLFILENAME,                                   "RM WARNING: null file name"},
        {RM_BADRECORDSIZE,                                  "RM WARNING: bad record size"},
        {RM_NOMEMORYLEFT,                                   "RM WARNING: no enough memory to alloc"},
        {RM_BITMAPISFULL,                                   "RM WARNING: bitmap is full"},
        {RM_RIDDELETED,                                     "RM WARNING: rid already deleted"},
        {RM_INVALIDFILHANDLE,                               "RM WARNING: invalid file handle"},
        {RM_INVALIDSCAN,                                    "RM WARNING: invalid file scan"},
        {RM_ENDOFPAGE,                                      "RM WARNING: already end of page"},

        {RM_TOOLARGERECSIZE,                                "RM ERROR: recordSize is too large for current pagefile system"},

        {SP_NULLFILENAME,                                   "SP WARNING: null file name"},
        {SP_UNIX,                                           "SP ERROR: linux file operation error"},

        {IX_EOF,                                            "IX WARNING: end of index, no more entry"},
        {IX_ALREADY_IN_BTREE,                               "IX WARNING: entry already in b+ tree"},
        {IX_INDEX_SCAN_CLOSE,                               "IX WARNING: index scan is closed cannot get next entry"},
        {IX_ALREADY_NOT_IN_BTREE,                           "IX WARNING: entry already not in b+ tree"},
        {IX_FIND_NON_EXIST,                                 "IX ERROR: found non-existent entry"},
        {IX_NOT_FIND,                                       "IX ERROR: not found entry"},
        {IX_INSERT_TWICE,                                   "IX ERROR: insert one entry twice"},
        {IX_DELETE_TWICE,                                   "IX ERROR: delete one entry twice"},
        {IX_DELETE_NON_EXIST,                               "IX ERROR: delete non exist entry"},

        {STAT_INVALID_ARGS,                                 "STAT ERROR: invalid args"},
        {STAT_UNKNOWN_KEY,                                  "STAT ERROR: unknown key"},

        {SM_INVALID_NAME,                                   "SM WARNING: INVALID NAME"},
        {SM_TABLE_ALREADY_IN,                               "SM WARNING: TABLE ALREADY IN DATABASE"},
        {SM_TABLE_IS_FULL,                                  "SM WARNING: TABLE IS FULL"},
        {SM_COLUMN_ALREADY_IN,                              "SM WARNING: COLUMN ALREADY IN TABLE"},
        {SM_COLUMN_IS_FULL,                                 "SM WARNING: COLUMN IS FULL"},
        {SM_DB_ALREADY_IN,                                  "SM WARNING: DATABASE ALREADY IN DBMS"},
        {SM_DB_NOT_EXIST,                                   "SM WARNING: DATABASE NOT EXIST"},
        {SM_DB_IS_FULL,                                     "SM WARNING: DATRABASE IS FULL"},
        {SM_NOT_USING_DATABASE,                             "SM WARNING: NOT USING DATABASE"},
        {SM_TABLE_NOT_EXIST,                                "SM WARNING: TABLE NOT EXIST IN DATABASE"},
        {SM_ALREADY_HAVE_PRIMARY_KEY,                       "SM WARNING: ALREADY HAVE PRIMARY KEY"},
        {SM_COLUMN_NOT_EXIST,                               "SM WARNING: COLUMN NOT EXIST IN TABLE"},
        {SM_FOREIGN_KEYS_AND_PRIMARY_KEYS_NOT_MATCH,
                                                            "SM WARNING: THE NUMBER OF FOREIGN KEYS AND THE NUMBER OF PRIMARY KEYS ARE NOT MATCH"},
        {SM_PRIMARY_KEY_NOT_EXIST,                          "SM WARNING: PRIMARY KEY NOT EXIST"},
        {SM_FOREIGN_KEYS_IS_FULL,                           "SM WARNING: FOREIGN KEYS IS FULL"},
        {SM_REFERENCE_IS_FULL,                              "SM WARNING: REFERENCE IS FULL"},
        {SM_FOREIGN_KEY_NOT_EXIST,                          "SM WARNING: FOREIGN KEY NOT EXIST"},
        {SM_TABLE_HAS_REFERENCE,                            "SM WARNING: TABLE HAS REFERENCE, CANN'T BE DELETED"},
        {SM_INDEX_IS_FULL,                                  "SM WARNING: INDEX IS FULL"},
        {SM_INDEX_NOT_ALLOW_DUPLICATE,                      "SM WARNING: INDEX NOT ALLOW DUPLICATE"},
        {SM_INDEX_COLUMN_NOT_ALLOW_NULL,                    "SM WARNING: INDEX COLUMN NOT ALLOW NULL"},
        {SM_INDEX_ALREADY_IN,                               "SM WARNING: INDEX ALREADY IN"},
        {SM_INDEX_NOT_EXIST,                                "SM WARNING: INDEX NOT EXIST"},
        {SM_PRIMARY_KEY_HAS_FOREIGN_KEY_DEPENDENCY,
                                                            "SM WARNING: PRIMARY KEY HAS FOREIGN KEY DEPENDENCY, CANT'T BE DELETE DIRECTLY"},
        {SM_CANNOT_DROP_SPECIAL_COLUMN,                     "SM WARNING: CANNOT DROP PRIMARY KEY COLUMN OR FOREIGN KEY COLUMN  OR INDEXED COLUMN"},
        {SM_FOREIGN_KEY_ALREADY_EXIST,                      "SM WARNING: FOREIGN KEY ALREADY EXIST"},
        {SM_NOT_ALLOW_ADD_SPECIAL_KEY,                      "SM WARNING: NOT ALOOW TO ADD PRIMARY KEY COLUMN OR FOREIGN KEY COLUMN"},

        {QL_COLUMNS_VALUES_DONT_MATCH,                      "QL WARNING: THE NUMBER OF COLUMNS AND VALUES DON'T MATCH"},
        {QL_COLUMN_NOT_ALLOW_NULL,                          "QL WARNING: COLUMN NOT ALLOW NULL"},
        {QL_INT_OUT_OF_RANGE,                               "QL WARNING: INPUT INT IS OUT OF RANGE"},
        {QL_INT_CONT_CONVERT_TO_INT,                        "QL WARNING: INPUT CONT CONVERT TO INT"},
        {QL_FLOAT_OUT_OF_RANGE,                             "QL WARNING: INPUT FLOAT IS OUT OF RANGE"},
        {QL_FLOAT_CONT_CONVERT_TO_FLOAT,                    "QL WARNING: INPUT CONT CONVERT TO FLOAT"},
        {QL_CHAR_TOO_LONG,                                  "QL WARNING: INPUT CHAR IS LONGER THAN ITS MAX LENGTH"},
        {QL_DATE_CONT_CONVERT_TO_DATE,                      "QL WARNING: INPUT CANT CONVERT TO DATE"},
        {QL_DATE_IS_NOT_VALID,                              "QL WARNING: INPUT DATE IS NOT VALID/ NOT EXIST"},
        {QL_DECIMAL_FORMAT_ERROR,                           "QL WARNING: DECIMAL FORMAT IS NOT RIGHT"},
        {QL_PRIMARY_KEY_DUPLICATE,                          "QL WARNING: PRIMARY KEY IS NOT UNIQUE, NOT ALLOW"},
        {QL_VARCHAR_TOO_LONG,                               "QL WARNING: INPUT VARCHAR IS LONGER THAN ITS MAX LENGTH"},
        {QL_FILE_NOT_EXIST,                                 "QL WARNING: FILE NOT EXIST"},
        {QL_UNSUPPORTED_OPERATION_TYPE,                     "QL WARNING: UNSUPPORTED OPERATION TYPE"},
        {QL_UNSUPPORTED_ASSIGN_TYPE,                        "QL WARNING: UNSUPPORTED ASSIGN TYPE"},
        {QL_NO_INDICATE_PRIMARY_KEY_EXIST,                  "QL WARNING: INDICATE PRIMARY KEY VALUE NOT EXIST"},
        {QL_INDICATE_PRIMARY_KEY_HAS_REFERENCE_FOREIGN_KEY, "QL WARNING: INDICATE PRIMARY KEY VALUE HAS REFERENCE FOREIGN KEY"},

        {SM_UNIX,                                           "SM ERROR: UNIX ERROR"},


        {UNKNOWN_RC,                                        "Sorry: we haven't name this warning or error"}
};

void printError(RC rc) {
    if (msg.find(rc) == msg.end()) {
        printf("\n%s", "Sorry. Rc is not recorded in note.");
    } else {
        printf("\n%s", msg[rc].data());
    }
}