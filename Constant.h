#ifndef PAGE_DEF
#define PAGE_DEF

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAXNAME       24                // maximum length of a relation
// or attribute name
#define MAXSTRINGLEN  255               // maximum length of a
// string-type attribute
#define MAXATTRS      40                // maximum number of attributes
// in a relation
// ALL_PAGES is defined and used by the ForcePages method defined in RM
// and PF layers
const int ALL_PAGES = -1;
#define PAGE_SIZE 8192 //一个页面中的字节数
#define PAGE_INT_NUM 2048 //一个页面中的整数个数
#define PAGE_SIZE_IDX 13 //页面字节数以2为底的指数
#define MAX_FMT_INT_NUM 128
//#define BUF_PAGE_NUM 65536
#define MAX_FILE_NUM 128
#define MAX_TYPE_NUM 256
#define CAP 60000 //缓存中页面个数上限
#define MOD 60000 //hash算法的模
#define IN_DEBUG 0
#define DEBUG_DELETE 0
#define DEBUG_ERASE 1
#define DEBUG_NEXT 1
#define MAX_COL_NUM 31 //一个表中列的上限
#define MAX_TB_NUM 31 //数据库中表的个数上限
#define RELEASE 1

#define MAX_CHAR_LENGTH 255


// Return Code
enum RC {
    OK_RC, //ok return code
    //Return code for PF
            PF_PAGEPINNED, // page pinned in buffer
    PF_PAGENOTINBUF, // page isn't pinned in buffer
    PF_INVALIDPAGE, // invalid page number
    PF_FILEOPEN, // file is open
    PF_CLOSEDFILE, // file is closed
    PF_PAGEFREE, // page already free
    PF_PAGEUNPINNED, // page already unpinned
    PF_EOF, // end of file
    PF_TOOSMALL, // Resize buffer too small

    PF_NOMEM,            // out of memory
    PF_NOBUF,           // out of buffer space
    PF_INCOMPLETEREAD,   // incomplete read of page from file
    PF_INCOMPLETEWRITE,  // incomplete write of page to file
    PF_HDRREAD,          // incomplete read of header from file
    PF_HDRWRITE,         // incomplete write of header to file

    // Internal PF errors:
            PF_PAGEINBUF,        // new allocated page already in buffer
    PF_HASHNOTFOUND,     // hash table entry not found
    PF_HASHPAGEEXIST,    // page already exists in hash table
    PF_INVALIDNAME,      // invalid file name
    PF_UNIX,// Unix error


    RM_EOF,
    RM_SLOTNOTINRANGE,
    RM_DELETEDRECORD,
    RM_NOMORERECODE,
    RM_NOTPROPERRECORD,
    RM_NOTPROPERRID,
    RM_SCANNOTOPEN,
    RM_SCANNOTCLOSE,
    RM_INVALIDRECORD,
    RM_INVALIDBITIND,
    RM_BITINUSE,
    RM_BITNOTINUSE,
    RM_INVALIDRID,
    RM_NULLFILENAME,
    RM_BADRECORDSIZE,
    RM_NOMEMORYLEFT,
    RM_BITMAPISFULL,
    RM_RIDDELETED,
    RM_INVALIDFILHANDLE,
    RM_INVALIDSCAN,
    RM_ENDOFPAGE,

    RM_TOOLARGERECSIZE,

    //return code for SP
            SP_UNIX,
    SP_NULLFILENAME,

    //return code for IX
            IX_EOF,
    IX_ALREADY_IN_BTREE,
    IX_ALREADY_NOT_IN_BTREE,
    IX_INDEX_SCAN_CLOSE,
    IX_FIND_NON_EXIST,
    IX_NOT_FIND,
    IX_INSERT_TWICE,
    IX_DELETE_TWICE,
    IX_DELETE_NON_EXIST,

    //return code for SM
            SM_INVALID_NAME,
    SM_TABLE_ALREADY_IN,
    SM_TABLE_IS_FULL,
    SM_COLUMN_ALREADY_IN,
    SM_COLUMN_IS_FULL,
    SM_DB_ALREADY_IN,
    SM_DB_NOT_EXIST,

    STAT_INVALID_ARGS,
    STAT_UNKNOWN_KEY,

    UNKNOWN_RC
};

struct Date {
    int year;
    short month;
    short day;
};
struct Varchar {
    int offset;
    int length;
};

enum AttrType {
    INT,
    FLOAT,
    STRING,
    DATE,
    VARCHAR
};

union AttrValue {
    int intVersion;
    float floatVersion;
    Date dateVersion;
    Varchar varcharVersion;
    char stringValue[MAX_CHAR_LENGTH + 1];
};


//
// Comparison operators
//
enum CompOp {
    NO_OP,                                      // no comparison
    EQ_OP, NE_OP, LT_OP, GT_OP, LE_OP, GE_OP    // binary atomic operators
};

//
// Pin Strategy Hint
//
enum ClientHint {
    NO_HINT                                     // default value
};

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#define TRY(_something) if (RC __rc = (_something)) return __rc;
#define CVOID(_x) (*(reinterpret_cast<char**>(&(_x))))


#endif