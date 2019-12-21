//
// Created by 赵鋆峰 on 2019/12/21.
//

#ifndef ROBODBMS_QL_TABLE_H
#define ROBODBMS_QL_TABLE_H


#include "../SM/SM_Constant.h"

class QL_Table {
public:
    explicit QL_Table(const TableMeta &tableMeta);

private:
    TableMeta tableMeta{};


};


#endif //ROBODBMS_QL_TABLE_H
