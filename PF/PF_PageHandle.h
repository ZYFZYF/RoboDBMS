//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBOSQL_PF_PAGEHANDLE_H
#define ROBOSQL_PF_PAGEHANDLE_H


#include "../Constant.h"
#include "PF_Constant.h"

class PF_PageHandle {
    friend class PF_FileHandle;

public:
    PF_PageHandle();                            // Default constructor
    ~PF_PageHandle();                            // Destructor

    // Copy constructor
    PF_PageHandle(const PF_PageHandle &pageHandle);

    // Overloaded =
    PF_PageHandle &operator=(const PF_PageHandle &pageHandle);

    RC GetData(char *&pData) const;           // Set pData to point to
    // the page contents
    RC GetPageNum(PageNum &pageNum) const;       // Return the page number
private:
    int pageNum;                                  // page number
    char *pPageData;                               // pointer to page data
};


#endif //ROBOSQL_PF_PAGEHANDLE_H
