//
// Created by 赵鋆峰 on 2019/11/12.
//

#include <fcntl.h>
#include <cstring>
#include "SP_Manager.h"
#include "../PF/PF_Constant.h"
#include "SP_Constant.h"

RC SP_Manager::CreateStringPool(const char *fileName) {
    //空文件名
    if (fileName == nullptr) {
        return SP_NULLFILENAME;
    }
    //创建文件
    int fd;
    if ((fd = open(fileName, O_CREAT | O_EXCL | O_WRONLY, CREATION_MASK)) < 0)
        return SP_UNIX;
    //初始化两个空白header，一个长度为0，另一个长度为无穷
    char spaceHeader[SP_SPACE_HEADER_SIZE * 2];
    memset(spaceHeader, 0, sizeof(spaceHeader));
    auto firstSpace = (SP_SpaceHeader *) spaceHeader;
    firstSpace->prevOffset = NO_MORE_SPACE;
    firstSpace->nextOffset = SP_SPACE_HEADER_SIZE;
    firstSpace->spaceLength = 0;
    auto secondSpace = (SP_SpaceHeader *) (spaceHeader + SP_SPACE_HEADER_SIZE);
    secondSpace->prevOffset = 0;
    secondSpace->nextOffset = NO_MORE_SPACE;
    secondSpace->spaceLength = INF_SPACE;
    //写入文件
    if (write(fd, spaceHeader, SP_SPACE_HEADER_SIZE * 2) != SP_SPACE_HEADER_SIZE * 2) {
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