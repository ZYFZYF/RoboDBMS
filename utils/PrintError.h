//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBODBMS_PRINTERROR_H
#define ROBODBMS_PRINTERROR_H

#include "../Constant.h"

#define DO(_something) if (RC __rc = (_something)) printError(__rc);

void printError(RC rc);

#endif //ROBODBMS_PRINTERROR_H
