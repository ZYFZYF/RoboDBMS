//
// Created by 赵鋆峰 on 2019/12/2.
//

#ifndef ROBODBMS_QL_MANAGER_H
#define ROBODBMS_QL_MANAGER_H


class QL_Manager {

public:
    QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);

    ~QL_Manager();                               // Destructor

    RC Select(int nSelAttrs,                   // # attrs in select clause
              const RelAttr selAttrs[],        // attrs in select clause
              int nRelations,                // # relations in from clause
              const char *const relations[],  // relations in from clause
              int nConditions,               // # conditions in where clause
              const Condition conditions[]);   // conditions in where clause

    RC Insert(const char *relName,             // relation to insert into
              int nValues,                   // # values
              const Value values[]);           // values to insert

    RC Delete(const char *relName,             // relation to delete from
              int nConditions,               // # conditions in where clause
              const Condition conditions[]);   // conditions in where clause

    RC Update(const char *relName,             // relation to update
              const RelAttr &updAttr,          // attribute to update
              const int bIsValue,              // 1 if RHS is a value, 0 if attr
              const RelAttr &rhsRelAttr,       // attr on RHS to set LHS eq to
              const Value &rhsValue,           // or value to set attr eq to
              int nConditions,               // # conditions in where clause
              const Condition conditions[]);   // conditions in where clause

};


#endif //ROBODBMS_QL_MANAGER_H
