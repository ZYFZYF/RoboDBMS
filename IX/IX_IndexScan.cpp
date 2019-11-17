//
// Created by 赵鋆峰 on 2019/11/16.
//

#include "IX_IndexScan.h"

IX_IndexScan::IX_IndexScan() {

}

IX_IndexScan::~IX_IndexScan() {

}

RC IX_IndexScan::OpenScan(const IX_IndexHandle &indexHandle, CompOp compOp, void *value, ClientHint pinHint) {
    return PF_NOBUF;
}

RC IX_IndexScan::GetNextEntry(RM_RID &rid) {
    return PF_NOBUF;
}

RC IX_IndexScan::CloseScan() {
    return PF_NOBUF;
}
