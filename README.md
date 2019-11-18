# RoboDBMS
## NOTICE
 - 注意区分有些RC，有些返回非0是正常的，这样数据库还应该继续工作，所以要注意处理后事
## TODO
 - RM
    - 支持变长字符串 √
    - Bitmap查找效率问题 ×
 - IX
    - 索引建立 √
    - 变长字符串做索引 × (不打算支持)
    - 支持日期类型并支持日期类型做索引 ×
 - 修改比较器的获取方式 √
 - 在比较器中方便地加入范围查找 ×
## Reference
 - https://github.com/huzecong/rebaseDB/tree/initial (Initial Redbase)
 - https://github.com/yifeih/redbase
 - https://github.com/parachvte/B-Plus-Tree
 
