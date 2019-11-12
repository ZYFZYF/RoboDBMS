//
// Created by 赵鋆峰 on 2019/11/12.
//

#include <fcntl.h>
#include <cstring>
#include <tclDecls.h>
#include "SP_Manager.h"
#include "../PF/def.h"
#include "def.h"

RC SP_Manager::CreateStringPool(const char *fileName) {
    //空文件名
    if (fileName == nullptr) {
        return SP_NULLFILENAME;
    }
    //创建文件
    int fd;
    if ((fd = open(fileName, O_CREAT | O_EXCL | O_WRONLY | CREATION_MASK)) < 0)
        return SP_UNIX;
    //初始化一个StringPool的header
    char header[SP_HEADER_SIZE + SP_SPACE_HEADER_SIZE];
    memset(header, 0, sizeof(header));
    auto spHeader = (SP_Header *) header;
    spHeader->firstSpaceOffset = spHeader->lastSpaceOffset = SP_HEADER_SIZE;
    auto spSpaceHeader = (SP_SpaceHeader *) (header + SP_HEADER_SIZE);
    spSpaceHeader->prevOffset = spSpaceHeader->nextOffset = NO_MORE_SPACE;
    spSpaceHeader->spaceLength = INF_SPACE;
    //写入文件
    if (write(fd, header, SP_HEADER_SIZE + SP_SPACE_HEADER_SIZE) != SP_HEADER_SIZE + SP_SPACE_HEADER_SIZE) {
        close(fd);
        return SP_UNIX;
    }
    //关闭文件
    if (close(fd) < 0)
        return SP_UNIX;

    return OK_RC;
}

RC SP_Manager::DestroyStringPool(const char *fileName) {
    if (fileName == nullptr) {
        return SP_NULLFILENAME;
    }
    if (unlink(fileName) < 0)
        return (PF_UNIX);
    return OK_RC;
}

RC SP_Manager::OpenStringPool(const char *fileName, SP_Handle &spHandle) {
    if (fileName == nullptr) {
        return SP_NULLFILENAME;
    }
    int fd;
    if ((fd = open(fileName, O_RDWR)) < 0) {
        return SP_UNIX;
    }
    spHandle.fd = fd;
    return OK_RC;
}

RC SP_Manager::CloseStringPool(SP_Handle &spHandle) {
    if (close(spHandle.fd) < 0) {
        return SP_UNIX;
    }
    return OK_RC;
}