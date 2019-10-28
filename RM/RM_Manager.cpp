//
// Created by 赵鋆峰 on 2019/10/27.
//

#include <cstring>
#include "RM_Manager.h"

RM_Manager::RM_Manager(PF_Manager &pfm) {
    this->pfm = pfm;
}

RM_Manager::~RM_Manager() {

}

RC RM_Manager::CreateFile(const char *fileName, int recordSize) {
    RC rc;
    //先检查recordSize是否合法，并计算每页可以放多少条记录
    if (fileName == nullptr) {
        return RM_NULLFILENAME;
    }
    if (recordSize <= 0 || recordSize > PF_PAGE_SIZE) {
        return RM_BADRECORDSIZE;
    }
    int recordNumPerPage = RM_FileHandle::CalcRecordNumPerPage(recordSize);
    if (recordNumPerPage == 0) {
        return RM_BADRECORDSIZE;
    }
    int bitMapSize = RM_FileHandle::ConvertBitToMultiBits(recordNumPerPage);
    int bitMapOffset = RM_PAGE_HEADER_SIZE;
    //创建文件
    if ((rc = pfm.CreateFile(fileName))) {
        return rc;
    }
    //打开文件，尝试初始化文件头
    PF_FileHandle pfh;
    if ((rc = pfm.OpenFile(fileName, pfh))) {
        return rc;
    }
    //分配文件头页用来初始化一些信息
    PF_PageHandle pgh;
    PageNum pageNum;
    if ((rc = pfh.AllocatePage(pgh)) || (rc = pgh.GetPageNum(pageNum))) {
        return rc;
    }
    char *data;
    pgh.GetData(data);
    //初始化文件头
    auto rfh = (RM_FileHeader *) data;
    rfh->recordSize = recordSize;
    rfh->recordNumPerPage = recordNumPerPage;
    rfh->bitMapOffset = bitMapOffset;
    rfh->bitMapSize = bitMapSize;
    //关闭文件
    if ((rc = pfh.MarkDirty(pageNum)) || (rc = pfh.UnpinPage(pageNum)) || (rc = pfm.CloseFile(pfh))) {
        return rc;
    }
    return OK_RC;
}

RC RM_Manager::DestroyFile(const char *fileName) {
    if (fileName == nullptr) {
        return RM_NULLFILENAME;
    }
    return pfm.DestroyFile(fileName);
}

RC RM_Manager::OpenFile(const char *fileName, RM_FileHandle &fileHandle) {
    RC rc;
    //打开文件，拿到PF_FileHandle
    PF_FileHandle pfh;
    if ((rc = pfm.OpenFile(fileName, pfh))) {
        return rc;
    }
    //拿到文件头页，PF_PageHandle
    PageNum pageNum;
    PF_PageHandle pph;
    if ((rc = pfh.GetFirstPage(pph)) || (rc = pph.GetPageNum(pageNum))) {
        pfh.UnpinPage(pageNum);
        pfm.CloseFile(pfh);
        return rc;
    }
    //把文件头页的信息写入RM_FileHandle中
    char *pageData;
    pph.GetData(pageData);
    auto rfh = (RM_FileHeader *) pageData;
    memcpy(&fileHandle.rfh, rfh, sizeof(struct RM_FileHeader));
    fileHandle.pfh = pfh;

    //释放文件头页
    if ((rc = pfh.UnpinPage(pageNum))) {
        pfm.CloseFile(pfh);
        return rc;
    }

    return OK_RC;

}

RC RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
    RC rc;
    if ((rc = pfm.CloseFile(fileHandle.pfh))) {
        return rc;
    }
    return OK_RC;
}
