# RoboDBMS
## NOTICE
 - 注意区分有些RC，有些返回非0是正常的，这样数据库还应该继续工作，所以要注意处理后事
## TODO
 - RM
    - 支持变长字符串 √
    - Bitmap查找效率问题 × (不是大问题，优先级往后放)
    - 析构函数的bug ×
 - IX
    - 索引建立 √
    - 变长字符串做索引 √ (~~不打算支持~~ 支持了_(:з」∠)_)
    - 变长字符串做索引速度过慢问题（频繁的打开、关闭、读文件）×
    - 支持日期类型并支持日期类型做索引 √
    - 支持联合索引 √
 - SM
    - 系统管理 √
    - 列增加和删除 ×
    - 支持联合主键 √
    - 支持联合外键 √
 - QL
    - 查询优化 ×
    - 多表连接 ×
    - 聚集查询 ×
    - 模糊查询 ×
    - 嵌套查询 ×
 - PS
    - 命令解析 部分完成
 - 其他
    - 修改比较器的获取方式 √
    - 在比较器中方便地加入范围查找 ×
    - 支持字符串的like表达式 ×
    - 修改项目结构 ×
    - 可能存在的内存泄漏问题（lex） ×
## TIPS
 - 序列化的是Varchar，但是内存中可以存成String
## Reference
 - https://github.com/huzecong/rebaseDB/tree/initial (Initial Redbase)
 - https://github.com/yifeih/redbase
 - https://github.com/parachvte/B-Plus-Tree
 
