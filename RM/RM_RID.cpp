//
// Created by 赵鋆峰 on 2019/10/27.
//

#include "RM_RID.h"

RM_RID::RM_RID() {

}

RM_RID::RM_RID(PageNum pageNum, SlotNum slotNum) {
    this->pageNum = pageNum;
    this->slotNum = slotNum;
}

RM_RID::~RM_RID() {

}

RC RM_RID::GetPageNum(PageNum &pageNum) const {
    RC rc;
    if ((rc = IsValidRID())) {
        return rc;
    }
    pageNum = this->pageNum;
    return OK_RC;
}

RC RM_RID::GetSlotNum(SlotNum &slotNum) const {
    RC rc;
    if ((rc = IsValidRID())) {
        return rc;
    }
    slotNum = this->slotNum;
    return OK_RC;
}

bool RM_RID::operator==(const RM_RID &rhs) const {
    return pageNum == rhs.pageNum &&
           slotNum == rhs.slotNum;
}

bool RM_RID::operator!=(const RM_RID &rhs) const {
    return !(rhs == *this);
}

RM_RID::RM_RID(const RM_RID &rmRid) {
    this->pageNum = rmRid.pageNum;
    this->slotNum = rmRid.slotNum;
}

RM_RID &RM_RID::operator=(const RM_RID &rmRid) {
    this->pageNum = rmRid.pageNum;
    this->slotNum = rmRid.slotNum;
    return *this;
}

RC RM_RID::IsValidRID() const {
    if (pageNum > 0 && slotNum >= 0) {
        return OK_RC;
    }
    return RM_INVALIDRID;
}

RC RM_RID::GetPageNumAndSlotNum(PageNum &pageNum, SlotNum &slotNum) const {
    RC rc;
    if ((rc = GetPageNum(pageNum)) || (rc = GetSlotNum(slotNum))) {
        return rc;
    }
    printf("return code is %d and page is %d, slot is %d\n", rc, pageNum, slotNum);
    return OK_RC;
}
