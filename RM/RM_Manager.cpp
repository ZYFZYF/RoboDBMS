//
// Created by 赵鋆峰 on 2019/10/27.
//

#include <cstring>
#include <iostream>
#include "RM_Manager.h"

RM_Manager::RM_Manager() : pfManager(PF_Manager::Instance()) {
}

RM_Manager::~RM_Manager() {

}

RC RM_Manager::CreateFile(const char *fileName, int recordSize) {
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
    TRY(pfManager.CreateFile(fileName));
    //打开文件，尝试初始化文件头
    PF_FileHandle pfh;
    TRY(pfManager.OpenFile(fileName, pfh));
    //分配文件头页用来初始化一些信息
    PF_PageHandle pgh;
    PageNum pageNum;
    TRY(pfh.AllocatePage(pgh));
    TRY(pgh.GetPageNum(pageNum));
    char *data;
    TRY(pgh.GetData(data));
    //初始化文件头
    auto rfh = (RM_FileHeader *) data;
    rfh->recordSize = recordSize;
    rfh->recordNumPerPage = recordNumPerPage;
    rfh->bitMapOffset = bitMapOffset;
    rfh->bitMapSize = bitMapSize;
    rfh->firstFreePage = NO_MORE_FREE_PAGE;
    rfh->pageCount = 1;
    // std::cerr << rfh->recordSize << ' ' << recordNumPerPage << ' ' << bitMapOffset << ' ' << bitMapSize << ' '
    //         << rfh->firstFreePage << ' ' << rfh->pageCount << std::endl;
    //关闭文件
    TRY(pfh.MarkDirty(pageNum));
    TRY(pfh.UnpinPage(pageNum));
    TRY(pfManager.CloseFile(pfh));
    return OK_RC;
}

RC RM_Manager::DestroyFile(const char *fileName) {
    if (fileName == nullptr) {
        return RM_NULLFILENAME;
    }
    return pfManager.DestroyFile(fileName);
}

RC RM_Manager::OpenFile(const char *fileName, RM_FileHandle &rmFileHandle) {
    RC rc;
    if (fileName == nullptr) {
        return RM_NULLFILENAME;
    }
    //原先的handle还没有释放
    if (rmFileHandle.isFileOpen) {
        return RM_INVALIDFILHANDLE;
    }
    //打开文件，拿到PF_FileHandle
    PF_FileHandle pfFileHandle;
    TRY(pfManager.OpenFile(fileName, pfFileHandle));
    //拿到文件头页，PF_PageHandle
    PageNum pageNum;
    PF_PageHandle pph;
    if ((rc = pfFileHandle.GetFirstPage(pph)) || (rc = pph.GetPageNum(pageNum))) {
        TRY(pfFileHandle.UnpinPage(pageNum));
        TRY(pfManager.CloseFile(pfFileHandle));
        return rc;
    }
    //把文件头页的信息写入RM_FileHandle中
    char *pageData;
    pph.GetData(pageData);
    auto rmFileHeader = (RM_FileHeader *) pageData;
    memcpy(&rmFileHandle.rmFileHeader, rmFileHeader, sizeof(struct RM_FileHeader));
    rmFileHandle.pfFileHandle = pfFileHandle;
    rmFileHandle.isFileOpen = true;
    rmFileHandle.isHeaderModified = false;

    //释放文件头页
    if ((rc = pfFileHandle.UnpinPage(pageNum))) {
        TRY(pfManager.CloseFile(pfFileHandle));
        return rc;
    }

    return OK_RC;

}

RC RM_Manager::CloseFile(RM_FileHandle &rmFileHandle) {
    PF_PageHandle pph;
    PageNum pageNum;
    char *data;
    //如果文件头变了的话，关闭文件的时候要写回
    //所以就需要保证任何时刻指向同一个文件的handle只有一个？
    if (rmFileHandle.isHeaderModified) {
        TRY(rmFileHandle.pfFileHandle.GetFirstPage(pph));
        TRY(pph.GetPageNum(pageNum));
        TRY(pph.GetData(data));
        memcpy(data, &rmFileHandle.rmFileHeader, sizeof(RM_FileHeader));
        TRY(rmFileHandle.pfFileHandle.MarkDirty(pageNum));
        TRY(rmFileHandle.pfFileHandle.UnpinPage(pageNum));
    }
    //关闭文件
    TRY(pfManager.CloseFile(rmFileHandle.pfFileHandle));
    //清除handle的标记
    if (!rmFileHandle.isFileOpen) {
        return RM_INVALIDFILHANDLE;
    }
    rmFileHandle.isFileOpen = false;
    return OK_RC;
}

RM_Manager &RM_Manager::Instance() {
    static RM_Manager instance;
    return instance;
}
