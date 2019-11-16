//
// Created by 赵鋆峰 on 2019/11/16.
//

#include <string>
#include "IX_Manager.h"

IX_Manager::IX_Manager(PF_Manager &pfm) {
    pfManager = pfm;
}

IX_Manager::~IX_Manager() {

}

RC IX_Manager::CreateIndex(const char *fileName, int indexNo, AttrType attrType, int attrLength) {
    //创建索引文件
    std::string indexFileName = getIndexFileName(fileName, indexNo);
    TRY(pfManager.CreateFile(indexFileName.c_str()));
    //尝试初始化
    PF_FileHandle pfFileHandle;
    PF_PageHandle pfPageHandle;
    PageNum pageNum;
    TRY(pfManager.OpenFile(indexFileName.c_str(), pfFileHandle));
    TRY(pfFileHandle.AllocatePage(pfPageHandle));
    TRY(pfPageHandle.GetPageNum(pageNum));
    //TODO  确定初始化的内容

    //标记为脏页并且取消pin
    TRY(pfFileHandle.MarkDirty(pageNum));
    TRY(pfFileHandle.UnpinPage(pageNum));
    //关闭文件
    TRY(pfManager.CloseFile(pfFileHandle));
    return OK_RC;
}

RC IX_Manager::DestroyIndex(const char *fileName, int indexNo) {
    std::string indexFileName = getIndexFileName(fileName, indexNo);
    TRY(pfManager.DestroyFile(indexFileName.c_str()));
    return OK_RC;
}

std::string IX_Manager::getIndexFileName(const char *fileName, int indexNo) const {
    std::string indexFileName = std::string(fileName) + "_index_" + std::to_string(indexNo);
    return indexFileName;
}

RC IX_Manager::OpenIndex(const char *fileName, int indexNo, IX_IndexHandle &indexHandle) {
    std::string indexFileName = getIndexFileName(fileName, indexNo);
    PF_FileHandle pfFileHandle;
    PF_PageHandle pfPageHandle;
    PageNum pageNum;
    TRY(pfManager.OpenFile(indexFileName.c_str(), pfFileHandle));
    TRY(pfFileHandle.GetFirstPage(pfPageHandle));
    TRY(pfPageHandle.GetPageNum(pageNum));
    //TODO 拿到IndexHandle的相关信息
    indexHandle.pfFileHandle = pfFileHandle;
    //把首页放回去
    TRY(pfFileHandle.UnpinPage(pageNum));
    return OK_RC;
}

RC IX_Manager::CloseIndex(IX_IndexHandle &indexHandle) {
    TRY(pfManager.CloseFile(indexHandle.pfFileHandle));
    return OK_RC;
}
