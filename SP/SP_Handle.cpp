//
// Created by 赵鋆峰 on 2019/11/12.
//

#include "SP_Handle.h"
#include "def.h"

SP_Handle::SP_Handle() {

}

SP_Handle::~SP_Handle() {

}

RC SP_Handle::InsertString(const char *string, int length, int &offset) {
    SP_Header spHeader;
    TRY(ReadHeader(spHeader));
    int nowSpaceOffset = spHeader.firstSpaceOffset;
    SP_SpaceHeader prevSpaceHeader, nowSpaceHeader, nextSpaceHeader;
    while (1) {
        ReadSpaceHeader(nowSpaceHeader, nowSpaceOffset);
        if (nowSpaceHeader.spaceLength > length) {
            //要看一下这个空还能不能剩下
            //不剩下一个头的大小了，整个给它
            if (nowSpaceHeader.spaceLength - length < SP_SPACE_HEADER_SIZE) {
                //前面还有空的话要改他的指针并写回到文件
                if (nowSpaceHeader.prevOffset != NO_MORE_SPACE) {
                    TRY(ReadSpaceHeader(prevSpaceHeader, nowSpaceHeader.prevOffset));
                    prevSpaceHeader.nextOffset = nowSpaceHeader.nextOffset;
                    TRY(WriteSpaceHeader(prevSpaceHeader, nowSpaceHeader.prevOffset));
                }
                //后面还有空的话要改他的指针并写回到文件
                if (nowSpaceHeader.nextOffset != NO_MORE_SPACE) {
                    TRY(ReadSpaceHeader(nextSpaceHeader, nowSpaceHeader.nextOffset));
                    nextSpaceHeader.prevOffset = nowSpaceHeader.prevOffset;
                    TRY(WriteSpaceHeader(nextSpaceHeader, nowSpaceHeader.nextOffset));
                }
                //最后用字符串把当前的空白覆盖
                TRY(WriteBuf(string, length, nowSpaceOffset));
            } else {
                int allocSize = ((length - 1) / SP_SPACE_HEADER_SIZE + 1) * SP_SPACE_HEADER_SIZE;
                //前面还有空的话要改他的指针并写回到文件
                if (nowSpaceHeader.prevOffset != NO_MORE_SPACE) {
                    TRY(ReadSpaceHeader(prevSpaceHeader, nowSpaceHeader.prevOffset));
                    prevSpaceHeader.nextOffset += allocSize;
                    TRY(WriteSpaceHeader(prevSpaceHeader, nowSpaceHeader.prevOffset));
                }
                //后面还有空的话要改他的指针并写回到文件
                if (nowSpaceHeader.nextOffset != NO_MORE_SPACE) {
                    TRY(ReadSpaceHeader(nextSpaceHeader, nowSpaceHeader.nextOffset));
                    nextSpaceHeader.prevOffset += allocSize;
                    TRY(WriteSpaceHeader(nextSpaceHeader, nowSpaceHeader.nextOffset));
                }
                //改当前空的长度并且写到另一个位置
                nowSpaceHeader.spaceLength -= allocSize;
                TRY(WriteSpaceHeader(nowSpaceHeader, nowSpaceOffset + allocSize));
                //最后用字符串把当前的空白覆盖
                TRY(WriteBuf(string, length, nowSpaceOffset));
            }
            offset = nowSpaceOffset;
            break;
        }
        nowSpaceOffset = nowSpaceHeader.nextOffset;
    }
}

RC SP_Handle::DeleteString(int offset, int length) {
    int allocSize = ((length - 1) / SP_SPACE_HEADER_SIZE + 1) * SP_SPACE_HEADER_SIZE;
    SP_Header spHeader;
    TRY(ReadHeader(spHeader));
    int nowSpaceOffset = spHeader.firstSpaceOffset;
    SP_SpaceHeader prevSpaceHeader, nowSpaceHeader, nextSpaceHeader;
    while (1) {
        ReadSpaceHeader(nowSpaceHeader, nowSpaceOffset);
        //发现下一个空白的开始已经超过了该字符串的开始位置，那么这个就一定是这个字符串前面的最后一块儿空白了
        if (nowSpaceHeader.nextOffset > offset) {
            //判断一下是否恰好与前一块儿空白相接
            if (nowSpaceOffset + nowSpaceHeader.spaceLength == offset) {
                nowSpaceHeader.spaceLength += allocSize;
                TRY(WriteSpaceHeader(nowSpaceHeader, nowSpaceOffset));
            } else {
                //在当前空白后新插一块儿空白
                //先修改当前空白
                int nextSpaceOffset = nowSpaceHeader.nextOffset;
                nowSpaceHeader.nextOffset = offset;
                TRY(WriteSpaceHeader(nowSpaceHeader, nowSpaceOffset));
                //插入新增的空白
                nowSpaceHeader.prevOffset = nowSpaceOffset;
                nowSpaceHeader.nextOffset = nextSpaceOffset;
                nowSpaceHeader.spaceLength = allocSize;
                nowSpaceOffset = offset;
                TRY(WriteSpaceHeader(nowSpaceHeader, nowSpaceOffset));
            }
            //看看是否产生了连锁反应与后面的相连
            if (nowSpaceOffset + nowSpaceHeader.spaceLength == nowSpaceHeader.nextOffset) {
                TRY(ReadSpaceHeader(nextSpaceHeader, nowSpaceHeader.nextOffset));
                nowSpaceHeader.spaceLength += nextSpaceHeader.spaceLength;
                nowSpaceHeader.nextOffset = nextSpaceHeader.nextOffset;
                WriteSpaceHeader(nowSpaceHeader, nowSpaceOffset);
            }
        }
        nowSpaceOffset = nowSpaceHeader.nextOffset;
    }
}

RC SP_Handle::UpdateString(const char *string, int length, int &offset, int old_length) {
    DeleteString(offset, old_length);
    InsertString(string, length, offset);
}

RC SP_Handle::GetStringData(char *data, int offset, int length) {
    TRY(ReadBuf(data, length, offset));
}

RC SP_Handle::ReadHeader(SP_Header &spHeader) {
    if (lseek(fd, 0, SEEK_SET) < 0) {
        return SP_UNIX;
    }
    if (read(fd, &spHeader, SP_HEADER_SIZE) != SP_HEADER_SIZE) {
        return SP_UNIX;
    }
    return OK_RC;
}

RC SP_Handle::WriteHeader(SP_Header spHeader) {
    if (lseek(fd, 0, SEEK_SET) < 0) {
        return SP_UNIX;
    }
    if (write(fd, &spHeader, SP_HEADER_SIZE) != SP_HEADER_SIZE) {
        return SP_UNIX;
    }
    return OK_RC;
}

RC SP_Handle::ReadSpaceHeader(SP_SpaceHeader &spSpaceHeader, int offset) {
    if (lseek(fd, offset, SEEK_SET)) {
        return SP_UNIX;
    }
    if (read(fd, &spSpaceHeader, SP_SPACE_HEADER_SIZE) != SP_SPACE_HEADER_SIZE) {
        return SP_UNIX;
    }
    return OK_RC;
}

RC SP_Handle::WriteSpaceHeader(SP_SpaceHeader spSpaceHeader, int offset) {
    if (lseek(fd, offset, SEEK_SET)) {
        return SP_UNIX;
    }
    if (write(fd, &spSpaceHeader, SP_SPACE_HEADER_SIZE) != SP_SPACE_HEADER_SIZE) {
        return SP_UNIX;
    }
    return OK_RC;
}

RC SP_Handle::ReadBuf(char *data, int length, int offset) {
    if (lseek(fd, offset, SEEK_SET)) {
        return SP_UNIX;
    }
    if (read(fd, data, length) != length) {
        return SP_UNIX;
    }
    return OK_RC;
}

RC SP_Handle::WriteBuf(const char *data, int length, int offset) {
    if (lseek(fd, offset, SEEK_SET)) {
        return SP_UNIX;
    }
    if (write(fd, data, length) < 0) {
        return SP_UNIX;
    }
    return OK_RC;
}