//
// Created by 赵鋆峰 on 2019/12/2.
//

#ifndef ROBODBMS_QL_MANAGER_H
#define ROBODBMS_QL_MANAGER_H


#include "../RM/RM_Manager.h"
#include "../IX/IX_Manager.h"
#include "../SM/SM_Manager.h"
#include "../PS/PS_Expr.h"

class QL_Manager {

public:
    static QL_Manager &Instance();

    ~QL_Manager();

    RC
    Select(std::vector<PS_Expr> *valueList, std::vector<TableMeta> *tableMetaList, std::vector<PS_Expr> *conditionList,
           std::vector<PS_Expr> *groupByList);

    //从文件一次性插入一大堆
    RC Insert(const char *tbName, const char *fileName);

    RC Insert(const char *tbName, std::vector<const char *> *columnList, std::vector<AttrValue> *constValueList);

    RC Delete(const char *tbName, std::vector<PS_Expr> *conditionList);

    RC Update(const char *tbName, std::vector<std::pair<std::string, PS_Expr> > *assignExprList,
              std::vector<PS_Expr> *conditionList);

    //输出表的行数
    RC Count(const char *tbName);

private:
    QL_Manager();

    SM_Manager smManager;
    IX_Manager ixManager;
    RM_Manager rmManager;

};


#endif //ROBODBMS_QL_MANAGER_H
