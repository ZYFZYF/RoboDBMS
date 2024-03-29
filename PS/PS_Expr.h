//
// Created by 赵鋆峰 on 2019/12/27.
//

#ifndef ROBODBMS_PS_EXPR_H
#define ROBODBMS_PS_EXPR_H


#include "../Attr.h"
#include "../SM/SM_Constant.h"
#include<map>

extern bool is_first_iteration_round;
extern int aggregation_count;


class SM_Table;

class PS_Expr {
    friend class SM_Table;

    friend class QL_MultiTable;

    friend class QL_Manager;

public:
    //叶子就是这六种类型
    explicit PS_Expr();

    explicit PS_Expr(bool _value);

    explicit PS_Expr(int _value);

    explicit PS_Expr(float _value);

    explicit PS_Expr(char *_value);

    PS_Expr(std::vector<PS_Expr> *_exprList);

    PS_Expr(PS_Expr *_left, Operator _op, bool _isAny, std::vector<PS_Expr> *_exprList);

    PS_Expr(const char *tbName, const char *colName);

    //运算得到的类型
    PS_Expr(PS_Expr *_left, Operator _op, PS_Expr *_right);

    RC eval(SM_Table &table, char *record);

    RC pushUp(std::string group = "NULL");

    //获取左边的值减去右边的值的差
    int cmp();

    void setName(const std::string &_name);

    std::string to_string();


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
    std::string name;
    int stringMaxLength{};
    //聚合相关
    int aggregationIndex{0};
    int updateCount{0}; //该分组已经有多少个元素了
    //存一个list，用来解决in（本来是一个right存一个表达式，有些情况右侧是多个表达式，那么弃用right，用这个list存）
    std::vector<PS_Expr> *exprList{nullptr};
    bool isAny{false};

    RC initAggregation(Operator op, PS_Expr *expr);

    RC updateAggregation(Operator op, PS_Expr *expr);

    bool isComparable();
};

extern std::map<std::pair<std::string, int>, PS_Expr> group_aggregation_expr;


#endif //ROBODBMS_PS_EXPR_H
