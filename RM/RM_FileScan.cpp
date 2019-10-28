//
// Created by 赵鋆峰 on 2019/10/27.
//

#include "RM_FileScan.h"

RM_FileScan::RM_FileScan() {

}

RM_FileScan::~RM_FileScan() {

}

RC
RM_FileScan::OpenScan(const RM_FileHandle &fileHandle, AttrType attrType, int attrLength, int attrOffset, CompOp compOp,
                      void *value, ClientHint pinHint) {
    return OK_RC;
}

RC RM_FileScan::GetNextRec(RM_Record &rec) {
    return OK_RC;
}

RC RM_FileScan::CloseScan() {
    return OK_RC;
}
