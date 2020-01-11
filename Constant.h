#ifndef PAGE_DEF
#define PAGE_DEF

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <exception>

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
#define MAX_VARCHAR_LENGTH 65536
#define MAX_NAME_LENGTH 30  //一切名字的最长长度
#define MAX_LINE_LENGTH 100000 //最长的一行长度
#define ATTR_TYPE_LENGTH (sizeof(AttrType))
#define MAX_AGGREGATION_COUNT 100
#define ATTR_ITEM_LENGTH (sizeof(AttrType) + sizeof(AttrValue))


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
    SM_UNIX,
    SM_DB_IS_FULL,
    SM_NOT_USING_DATABASE,
    SM_TABLE_NOT_EXIST,
    SM_ALREADY_HAVE_PRIMARY_KEY,
    SM_COLUMN_NOT_EXIST,
    SM_FOREIGN_KEYS_AND_PRIMARY_KEYS_NOT_MATCH,
    SM_PRIMARY_KEY_NOT_EXIST,
    SM_FOREIGN_KEYS_IS_FULL,
    SM_REFERENCE_IS_FULL,
    SM_FOREIGN_KEY_NOT_EXIST,
    SM_TABLE_HAS_REFERENCE,
    SM_INDEX_IS_FULL,
    SM_INDEX_NOT_ALLOW_DUPLICATE,
    SM_INDEX_COLUMN_NOT_ALLOW_NULL,
    SM_INDEX_ALREADY_IN,
    SM_INDEX_NOT_EXIST,
    SM_PRIMARY_KEY_HAS_FOREIGN_KEY_DEPENDENCY,
    SM_FOREIGN_KEY_ALREADY_EXIST,
    SM_NOT_ALLOW_ADD_SPECIAL_KEY,
    SM_CANNOT_DROP_SPECIAL_COLUMN,

    QL_COLUMNS_VALUES_DONT_MATCH,
    QL_COLUMN_NOT_ALLOW_NULL,
    QL_INT_OUT_OF_RANGE,
    QL_INT_CONT_CONVERT_TO_INT,
    QL_FLOAT_OUT_OF_RANGE,
    QL_FLOAT_CONT_CONVERT_TO_FLOAT,
    QL_CHAR_TOO_LONG,
    QL_DATE_CONT_CONVERT_TO_DATE,
    QL_DATE_IS_NOT_VALID,
    QL_DECIMAL_FORMAT_ERROR,
    QL_PRIMARY_KEY_DUPLICATE,
    QL_VARCHAR_TOO_LONG,
    QL_FILE_NOT_EXIST,
    QL_UNSUPPORTED_OPERATION_TYPE,
    QL_UNSUPPORTED_ASSIGN_TYPE,
    QL_NO_INDICATE_PRIMARY_KEY_EXIST,
    QL_INDICATE_PRIMARY_KEY_HAS_REFERENCE_FOREIGN_KEY,

    STAT_INVALID_ARGS,
    STAT_UNKNOWN_KEY,

    UNKNOWN_RC
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
