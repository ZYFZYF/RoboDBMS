//
// Created by 赵鋆峰 on 2019/10/28.
//

#ifndef ROBODBMS_RM_CONSTANT_H
#define ROBODBMS_RM_CONSTANT_H

#include "../PF/PF_Constant.h"

struct RM_FileHeader {
    int recordSize;
    int recordNumPerPage;
    int bitMapOffset;
    int bitMapSize;
    PageNum firstFreePage;
    int pageCount;
};

struct RM_PageHeader {
    int recordNum;
    PageNum nextFreePage;
};

typedef char MultiBits;
#define NO_MORE_FREE_PAGE -1

#define RM_FILE_HEADER_SIZE sizeof(RM_FileHeader)
#define RM_PAGE_HEADER_SIZE sizeof(RM_PageHeader)

#endif //ROBODBMS_RM_CONSTANT_H
