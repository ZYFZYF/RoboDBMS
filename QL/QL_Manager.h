//
// Created by 赵鋆峰 on 2019/12/2.
//

#ifndef ROBODBMS_QL_MANAGER_H
#define ROBODBMS_QL_MANAGER_H


#include "../RM/RM_Manager.h"
#include "../IX/IX_Manager.h"
#include "../SM/SM_Manager.h"

class QL_Manager {

public:
    static QL_Manager &Instance();

    ~QL_Manager();

    RC Select();

    RC Insert();

    RC Delete();

    RC Update();

private:
    QL_Manager();

    SM_Manager smManager;
    IX_Manager ixManager;
    RM_Manager rmManager;

};


#endif //ROBODBMS_QL_MANAGER_H
