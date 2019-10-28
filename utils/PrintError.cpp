//
// Created by 赵鋆峰 on 2019/10/27.
//
#include "../def.h"
#include <map>
#include <string>
#include <iostream>
#include "PrintError.h"

using namespace std;

map<RC, string> msg = {
        {OK_RC,                 "OK RETURN CODE"},

        {PF_PAGEPINNED,         "PF WARNING: page pinned in buffer"},
        {PF_PAGENOTINBUF,       "PF WARNING: page is not in the buffer"},
        {PF_INVALIDPAGE,        "PF WARNING: invalid page number"},
        {PF_FILEOPEN,           "PF WARNING: file open"},
        {PF_CLOSEDFILE,         "PF WARNING: invalid file descriptor (file closed)"},
        {PF_PAGEFREE,           "PF WARNING: page already free"},
        {PF_PAGEUNPINNED,       "PF WARNING: page already unpinned"},
        {PF_EOF,                "PF WARNING: end of file"},
        {PF_TOOSMALL,           "PF WARNING: attempting to resize the buffer too small"},

        {PF_NOMEM,              "PF ERROR: no memory"},
        {PF_NOBUF,              "PF ERROR: no buffer space"},
        {PF_INCOMPLETEREAD,     "PF ERROR: incomplete read of page from file"},
        {PF_INCOMPLETEWRITE,    "PF ERROR: incomplete write of page to file"},
        {PF_HDRREAD,            "PF ERROR: incomplete read of header from file"},
        {PF_HDRWRITE,           "PF ERROR: incomplete write of header from file"},
        {PF_PAGEINBUF,          "PF ERROR: new page to be allocated already in buffer"},
        {PF_HASHNOTFOUND,       "PF ERROR: hash table entry not found"},
        {PF_HASHPAGEEXIST,      "PF ERROR: page already in hash table"},
        {PF_INVALIDNAME,        "PF ERROR: invalid file name"},

        {RM_EOF,                "RM WARNING: file is not opened"},
        {RM_SLOTNOTINRANGE,     "RM WARNING: SlotNum is out of range"},
        {RM_DELETEDRECORD,      "RM WARNING: record is already deleted"},
        {RM_NOMORERECODE,       "RM WARNING: no more records in scan"},
        {RM_NOTPROPERRECORD,    "RM WARNING: Record is not properly initialized"},
        {RM_NOTPROPERRID,       "RM WARNING: RID is not properly initialized"},
        {RM_SCANNOTOPEN,        "RM WARNING: scan is not opened"},
        {RM_SCANNOTCLOSE,       "RM WARNING: last opened scan is not closed"},
        {RM_INVALIDRECORD,      "RM WARNING: invalid record page"},
        {x:RM_INVALIDBITIND,    "RM WARNING: bit int is out of range"},
        {RM_BITINUSE,           "RM WARNING: bit already in use"},
        {RM_BITNOTINUSE,        "RM WARNING: bit not in use"},
        {RM_INVALIDRID,         "RM WARNING: invalid record id"},
        {RM_NULLFILENAME,       "RM WARNING: null file name"},
        {RM_BADRECORDSIZE,      "RM WARNING: bad record size"},
        {RM_NOMEMORYLEFT,       "RM WARNING: no enough memory to alloc"},

        {RM_TOOLARGERECORDSIZE, "RM ERROR: recordSize is too large for current pagefile system"},

        {STAT_INVALID_ARGS,     "STAT ERROR: invalid args"},
        {STAT_UNKNOWN_KEY,      "STAT ERROR: unknown key"},

        {UNKNOWN_RC,            "Sorry: we haven't name this warning or error"}
};

void printError(RC rc) {
    if (msg.find(rc) == msg.end()) {
        cerr << "Sorry. Rc is not recorded in note." << endl;
    } else {
        cerr << msg[rc] << endl;
    }
}