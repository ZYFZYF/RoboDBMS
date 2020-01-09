//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBODBMS_RM_RID_H
#define ROBODBMS_RM_RID_H

#include "../Constant.h"

typedef int PageNum;
typedef int SlotNum;

class RM_RID {
public:
    RM_RID();                                         // Default constructor
    RM_RID(PageNum pageNum, SlotNum slotNum);

    RM_RID(const RM_RID &rmRid);

    RM_RID &operator=(const RM_RID &rmRid);

    bool operator==(const RM_RID &rhs) const;

    bool operator!=(const RM_RID &rhs) const;

    bool operator<(const RM_RID &rhs) const;

    bool operator<=(const RM_RID &rhs) const;

    bool operator>(const RM_RID &rhs) const;

    bool operator>=(const RM_RID &rhs) const;


    ~RM_RID();

    // Destructor

    RC GetPageNum(PageNum &pageNum) const;         // Return page number
    RC GetSlotNum(SlotNum &slotNum) const;         // Return slot number
    RC GetPageNumAndSlotNum(PageNum &pageNum, SlotNum &slotNum) const;

    RC IsValidRID() const;

private:
    PageNum pageNum;
    SlotNum slotNum;
public:
    PageNum getPageNum() const;

    SlotNum getSlotNum() const;
};


#endif //ROBODBMS_RM_RID_H
