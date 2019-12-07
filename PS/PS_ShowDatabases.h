//
// Created by 赵鋆峰 on 2019/12/7.
//

#ifndef ROBODBMS_PS_SHOWDATABASES_H
#define ROBODBMS_PS_SHOWDATABASES_H


#include "PS_Node.h"

class PS_ShowDatabases : public PS_Node {
public:
    PS_ShowDatabases() = default;

    void run() override;
};


#endif //ROBODBMS_PS_SHOWDATABASES_H
