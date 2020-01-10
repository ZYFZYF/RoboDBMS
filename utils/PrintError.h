//
// Created by 赵鋆峰 on 2019/10/27.
//

#ifndef ROBODBMS_PRINTERROR_H
#define ROBODBMS_PRINTERROR_H

#include "../Constant.h"

#define DO(_something) try{if (RC __rc = (_something)) printError(__rc);}catch(const char* str){printf("%s\n",str);}catch(std::string &str){std::cout<<str<<std::endl;}

void printError(RC rc);

#endif //ROBODBMS_PRINTERROR_H
