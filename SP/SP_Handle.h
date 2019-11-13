//
// Created by 赵鋆峰 on 2019/11/12.
//

#ifndef ROBODBMS_SP_HANDLE_H
#define ROBODBMS_SP_HANDLE_H

#include "../def.h"

#define SP_SPACE_HEADER_SIZE sizeof(struct SP_SpaceHeader)

//分配给Varchar的空间一定是SP_SPACE_HEADER_SIZE的倍数
struct SP_SpaceHeader {
    int prevOffset;
    int nextOffset;
    int spaceLength;
};


class SP_Handle {
    friend class SP_Manager;

public:
    SP_Handle();

    ~SP_Handle();

    RC InsertString(const char *string, int length, int &offset);

    RC DeleteString(int offset, int length);

    RC UpdateString(const char *string, int length, int &offset, int old_length);

    RC GetStringData(char *data, int offset, int length);

    int fd;

//private:

    RC ReadSpaceHeader(SP_SpaceHeader &spSpaceHeader, int offset);

    RC WriteSpaceHeader(SP_SpaceHeader spSpaceHeader, int offset);

    RC ReadBuf(char *data, int length, int offset);

    RC WriteBuf(const char *data, int length, int offset);
};


#endif //ROBODBMS_SP_HANDLE_H
