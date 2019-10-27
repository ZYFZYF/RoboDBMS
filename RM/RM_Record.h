//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBODBMS_RM_RECORD_H
#define ROBODBMS_RM_RECORD_H


#include "RM_RID.h"

class RM_Record {
public:
    RM_Record();

    ~RM_Record();

    RM_Record(RM_RID &rmRid, char *bufferData, int size);

    RM_Record(const RM_Record &rmRecord);

    RM_Record &operator=(const RM_Record &rmRecord);

    // Return the data corresponding to the record.  Sets *pData to the
    // record contents.
    RC GetData(char *&pData) const;

    // Return the RID associated with the record
    RC GetRid(RM_RID &rid) const;

private:
    RM_RID rmRid;
    char *data;
    int size;
};


#endif //ROBODBMS_RM_RECORD_H
