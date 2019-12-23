//
// Created by 赵鋆峰 on 2019/10/27.
//

#include <cstring>
#include <cstdlib>
#include <iostream>
#include "RM_FileScan.h"
#include "../utils/Utils.h"

RM_FileScan::RM_FileScan() {
    openScan = false; // initially a filescan is not valid
    value = nullptr;
    initializedValue = false;
    hasPagePinned = false;
    scanEnded = true;
}

RM_FileScan::~RM_FileScan() {
    if (!scanEnded && hasPagePinned && openScan) {
        rmFileHandle->pfFileHandle.UnpinPage(pageNum);
    }
    if (initializedValue) {
        free(value);
        initializedValue = false;
    }
}

RC RM_FileScan::OpenScan(const RM_FileHandle &rmFileHandle,
                         AttrType attrType,
                         int attrLength,
                         int attrOffset,
                         CompOp compOp,
                         void *value) {
    if (openScan)
        return (RM_INVALIDSCAN);
    this->rmFileHandle = const_cast<RM_FileHandle *>(&rmFileHandle);
    this->value = nullptr;
    this->compOp = compOp;

    int recSize = (this->rmFileHandle)->rmFileHeader.recordSize;
    if (this->compOp != NO_OP) {
        if ((attrOffset + attrLength) > recSize || attrOffset < 0 || attrOffset > MAXSTRINGLEN)
            return (RM_INVALIDSCAN);
        this->attrOffset = attrOffset;
        this->attrLength = attrLength;
        if (attrType == FLOAT || attrType == INT) {
            if (attrLength != 4)
                return (RM_INVALIDSCAN);
            this->value = (void *) malloc(4);
            memcpy(this->value, value, 4);
            initializedValue = true;
        } else if (attrType == STRING) {
            this->value = (void *) malloc(attrLength);
            memcpy(this->value, value, attrLength);
            initializedValue = true;
        } else {
            return (RM_INVALIDSCAN);
        }
        this->attrType = attrType;
    }

    openScan = true;
    scanEnded = false;
    numTotalOnPage = 0;
    numSeenOnPage = 0;
    useNextPage = true;
    pageNum = 0;
    slotNum = BEGIN_SCAN;
    numSeenOnPage = 0;
    hasPagePinned = false;
    return OK_RC;
}

//从pph中获取这一页的记录数
RC RM_FileScan::GetRecordNumOnPage(PF_PageHandle &pfPageHandle, int &recordNum) {
    char *bitmap;
    struct RM_PageHeader *rph;
    TRY((this->rmFileHandle)->GetPageHeaderAndBitmap(pfPageHandle, rph, bitmap));
    recordNum = rph->recordNum;
    return OK_RC;
}

RC RM_FileScan::GetNextRec(RM_Record &rec) {
    if (scanEnded)
        return (RM_EOF);
    if (!openScan)
        return (RM_INVALIDSCAN);
    hasPagePinned = true;

    RC rc;
    while (true) {
        RM_Record tempRec;
        if ((rc = rmFileHandle->GetNextRecord(pageNum, slotNum, tempRec, currentPH, useNextPage))) {
            if (rc == RM_EOF) {
                hasPagePinned = false;
                scanEnded = true;
            }
            return rc;
        }
        hasPagePinned = true;
        if (useNextPage) {
            GetRecordNumOnPage(currentPH, numTotalOnPage);
            useNextPage = false;
            numSeenOnPage = 0;
            if (numTotalOnPage == 1)
                currentPH.GetPageNum(pageNum);
        }
        numSeenOnPage++;
        if (numTotalOnPage == numSeenOnPage) {
            useNextPage = true;
            TRY(rmFileHandle->pfFileHandle.UnpinPage(pageNum));
            hasPagePinned = false;
        }
        //从record中拿到RID并拿到存储位置和数据
        RM_RID rid;
        tempRec.GetRid(rid);
        rid.GetPageNumAndSlotNum(pageNum, slotNum);

        char *pData;
        TRY(tempRec.GetData(pData));
        //有比较函数需要结果为真才能返回
        if (compOp != NO_OP) {
            bool satisfies = Utils::Compare(pData + attrOffset, this->value, attrType, attrLength, compOp);
            //bool satisfies = (*comparator)(pData + attrOffset, this->value, attrType, attrLength);
            if (satisfies) {
                rec = tempRec;
                break;
            }
        } else {
            rec = tempRec;
            break;
        }
    }
    return OK_RC;
}

RC RM_FileScan::CloseScan() {
    if (!openScan) {
        return (RM_INVALIDSCAN);
    }
    if (hasPagePinned) {
        TRY(rmFileHandle->pfFileHandle.UnpinPage(pageNum));
    }
    if (initializedValue) {
        free(this->value);
        initializedValue = false;
    }
    openScan = false;
    return OK_RC;
}
