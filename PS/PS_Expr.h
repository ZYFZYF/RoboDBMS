//
// Created by 赵鋆峰 on 2019/12/27.
//

#ifndef ROBODBMS_PS_EXPR_H
#define ROBODBMS_PS_EXPR_H


#include "../Attr.h"
#include "../SM/SM_Constant.h"

class SM_Table;

class PS_Expr {
    friend class SM_Table;

public:
    //叶子就是这六种类型
    explicit PS_Expr();

    explicit PS_Expr(bool _value);

    explicit PS_Expr(int _value);

    explicit PS_Expr(float _value);

    explicit PS_Expr(char *_value);

    PS_Expr(char *tbName, char *colName);

    //运算得到的类型
    PS_Expr(PS_Expr *_left, Operator _op, PS_Expr *_right);

    RC eval(SM_Table &table, char *record);

    RC pushUp();

    //获取左边的值减去右边的值的差
    int cmp();


private:
    //是否是常量，无需和列数据挂钩
    bool isConst{false};
    //是否是一列中的数据
    bool isColumn{false};
    std::string tableName{};
    std::string columnName{};
    //数据类型
    AttrType type;
    AttrValue value{};
    //暂时把字符串相关的存到string里面
    std::string string;
    //二元运算
    PS_Expr *left{nullptr}, *right{nullptr};
    Operator op{NO_OP};
};

bool isComparable(AttrType type1, AttrType type2);


#endif //ROBODBMS_PS_EXPR_H
