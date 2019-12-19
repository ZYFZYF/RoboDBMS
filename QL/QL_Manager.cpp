//
// Created by 赵鋆峰 on 2019/12/2.
//

#include "QL_Manager.h"

QL_Manager &QL_Manager::Instance() {
    static QL_Manager instance;
    return instance;
}

QL_Manager::~QL_Manager() {

}

RC QL_Manager::Select() {
    return PF_EOF;
}

RC QL_Manager::Insert() {
    return PF_EOF;
}

RC QL_Manager::Delete() {
    return PF_EOF;
}

RC QL_Manager::Update() {
    return PF_EOF;
}

QL_Manager::QL_Manager() : rmManager(RM_Manager::Instance()), ixManager(IX_Manager::Instance()),
                           smManager(SM_Manager::Instance()) {

}
