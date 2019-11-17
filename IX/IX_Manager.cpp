//
// Created by 赵鋆峰 on 2019/11/16.
//

#include <string>
#include <cstring>
#include "IX_Manager.h"
#include "def.h"

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
    PF_PageHandle pfHeaderPageHandle, pfRootPageHandle;
    PageNum headerPageNum, rootPageNum;
    char *pageData;
    //打开文件
    TRY(pfManager.OpenFile(indexFileName.c_str(), pfFileHandle));
    //先分配头页
    TRY(pfFileHandle.AllocatePage(pfHeaderPageHandle));
    TRY(pfHeaderPageHandle.GetPageNum(headerPageNum));
    //再分配根页
    TRY(pfFileHandle.AllocatePage(pfRootPageHandle));
    TRY(pfRootPageHandle.GetPageNum(rootPageNum));
    //初始化索引文件头页并写入页数据
    IX_FileHeader ixFileHeader;
    ixFileHeader.attrType = attrType;
    ixFileHeader.attrLength = attrLength;
    ixFileHeader.maxKeyNum =
            (PF_PAGE_SIZE - IX_FILE_HEADER_SIZE) / (ixFileHeader.attrLength + VALUE_LENGTH + CHILD_LENGTH);
    ixFileHeader.firstKeyOffset = IX_BPLUS_TREE_NODE_SIZE;
    ixFileHeader.firstValueOffset = ixFileHeader.firstKeyOffset + ixFileHeader.attrLength * ixFileHeader.maxKeyNum;
    ixFileHeader.firstChildOffset = ixFileHeader.firstValueOffset + VALUE_LENGTH * ixFileHeader.maxKeyNum;
    ixFileHeader.rootPageNum = rootPageNum;
    TRY(pfHeaderPageHandle.GetData(pageData));
    memcpy(pageData, &ixFileHeader, IX_FILE_HEADER_SIZE);
    //初始化B+树根页并写入页数据
    IX_BPlusTreeNode ixBPlusTreeNode;
    ixBPlusTreeNode.isLeaf = true;
    ixBPlusTreeNode.isRoot = true;
    ixBPlusTreeNode.keyNum = 0;
    ixBPlusTreeNode.prev = ixBPlusTreeNode.next = ixBPlusTreeNode.father = NULL_NODE;
    TRY(pfRootPageHandle.GetData(pageData));
    memcpy(pageData, &ixBPlusTreeNode, IX_BPLUS_TREE_NODE_SIZE);
    //标记为脏页并且取消pin
    TRY(pfFileHandle.MarkDirty(headerPageNum));
    TRY(pfFileHandle.UnpinPage(headerPageNum));
    TRY(pfFileHandle.MarkDirty(rootPageNum));
    TRY(pfFileHandle.UnpinPage(rootPageNum));
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
    //拿到IndexHandle的相关信息
    indexHandle.pfFileHandle = pfFileHandle;
    char *pageData;
    TRY(pfPageHandle.GetData(pageData));
    memcpy(&(indexHandle.ixFileHeader), pageData, IX_FILE_HEADER_SIZE);
    //把首页放回去
    TRY(pfFileHandle.UnpinPage(pageNum));
    return OK_RC;
}

RC IX_Manager::CloseIndex(IX_IndexHandle &indexHandle) {
    TRY(pfManager.CloseFile(indexHandle.pfFileHandle));
    return OK_RC;
}
