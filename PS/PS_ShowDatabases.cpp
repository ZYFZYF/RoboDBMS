//
// Created by 赵鋆峰 on 2019/12/7.
//

#include "PS_ShowDatabases.h"
#include "../SM/SM_Manager.h"

void PS_ShowDatabases::run() {
    SM_Manager::Instance().ShowDatabases();
}
