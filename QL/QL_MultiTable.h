//
// Created by 赵鋆峰 on 2019/12/31.
//

#ifndef ROBODBMS_QL_MULTITABLE_H
#define ROBODBMS_QL_MULTITABLE_H


#include "../SM/SM_Table.h"

class QL_MultiTable {
public:
    explicit QL_MultiTable(std::vector<TableMeta> *tableMetaList);

    TableMeta select(std::vector<PS_Expr> *valueList, std::vector<PS_Expr> *conditionList);

    //从表名和列名获得表以及列id
    std::pair<SM_Table *, ColumnId> getColumn(std::string &tbName, std::string &columnName);

private:
    int tableNum;
    std::vector<SM_Table> tableList;
};


#endif //ROBODBMS_QL_MULTITABLE_H
