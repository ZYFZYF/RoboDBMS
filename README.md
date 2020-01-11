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
    - 列增加和删除 √
        - 这样一改整个记录得重改，然后整个索引也得重建？好像只要遍历一遍，一条一条加就可以了
        - 列增加，赠完之后改名的措施对于varchar怎么办
    - 列修改 √
    - 表更名 √
    - 完整性约束 √
        - 主键有外链时不能删除（包括每条记录和主键）
        - 外链不存在主键时不能插入（包括每条记录）
        - 主键不存在时删除主键
    - 支持联合主键 √
    - 支持联合外键 √
 - QL
    - 插入 √
        - 需要保证主键的唯一性，也是实现判断 这个主键是否存在 √
        - 同时记得更新索引 √
        - 常量转记录类型（记录转常量类型？也是需要的，因为有时候从表里拿出来的数据需要parse或者比较）√
    - 删除 √
        - 如果删除一条主键，它存在依赖，那么我们可以直接禁止，而不是级联 √
        - 需要一个判断 某个表里是否存在reference这个主键的外键 的函数 √
        - 记得删除索引 √
        - whereClause 
            - 需要支持字符串和date进行比较 √
            - 需要支持+-法和*/法的优先级问题 √
            - 需要进行const的传递 √
            - 需要支持小括号 √
            - 需要考虑插入负数的问题 √
                -  可以区分负号和减，负号需要紧挨数字，-和数字之间要有空格
    - 更新 √
        - setClause √
            - 这个比较容易，因为只有赋值操作 √
            - 需要考虑赋值为null的操作 √
                - 文件中的null不予识别 √
        - whereClause √
            - 删除和更新的应该是一样的 op 右边只能是常量(不对，是可以是列名的) √
                - 这部分需要一个  遍历整个表，进一步还能够根据索引来优化直接解决和常量比较的条件 
        - 更新要做成先删除要插入嘛？√
            - 需要根据是否牵扯到主键来区分对待，修改牵扯到了那么就必须先删除再增加，否则是原地更新，无需动主键（因为删除主键可能会失败）
    - 查询 √
        - whereClause
            - 右边还可以是列名
            - 只和常量比较的分到表内部做 然后多表之间的再暴力比较？
        - 查询出来的感觉是个表，然后有个record内容的vector？所以他也需要有列的定义 它也有tableMeta就可以了？
            - 查出来的直接存到一个表里面（createName可以取时间戳，或者固定的），有as语句的话是固定的
        - 可以支持Limit（只需要把输出信息控制一下就行了，前提是要实现order by（也可以不实现），limit有个偏移量）
        - 要每次select之后保证把临时的表清除掉
        - 单表
    - 查询优化 √
        - 用索引就算？所谓的DAG是什么？
        - 用索引直接给出一个RID的list如何？后面就直接用这个list去一个个拿以及判断了，我觉得可以，把record直接存下来不太现实，内存原因
        - 首先单维的支持各种comp，多维的只支持==
        - 多表连接的优化
            - 可以考虑每次循环的时候重拷贝一遍condition，然后每枚举一下eval一个，后面的就可以用前面得到的值利用索引进行检索了
    - 多表连接 √
        - 和双表可能差别不大？
        - 这里要解决一个问题就是不知道表的个数怎么写循环代码
    - 聚集查询 √
        - 怎么做？MIN/MAX/AVG/SUM/COUNT还可以  GROUP BY/HAVING 怎么办？
        - MIN/MAX是可以做到数值以及字符串类型上的，AVG/SUM只能在数值上，COUNT都可以（count可以再支持一下distinct？）
        - 支持一下group_concat？就是把一串的连起来，但是这样没法定义长度了（可以默认长度，比方说1024对齐mysql）×
        - 问题是group by的hash怎么做，怎么做到支持不同类型的hash，全部转成字符串么
            - 全部转成字符串然后变成std::string->PS_Expr的map，这样怎么拿回group的字段的值呢？好像不需要拿？或者反正每一行都有，可以自己拿
        - order by要支持吗 
            - 怎么支持？在列上建索引，然后遍历输出？我觉得可以
            - 还可以支持desc incs（但是只能支持整体一致的顺序，要么从大到小，要么从小到大）
        - 代码怎么实现
            - 要支持 MIN(age) + MAX(age)，MIN(age + 1)这种实现？其实是可以的，中间eval的时候，到了这一步停住做一个贡献（怎么做贡献？，然后最后整个算完之后再遍历一遍？   
            - 要么就是有group by的select和没有的直接分开计算，因为是两种模式
                - 没有的就是普通的eval
                - 有的话我们需要一个累加然后最后算完了再插入进去的机制？可以多来一次遍历，最开始拿到所有表达式的时候遍历一遍领取一个编号，然后形成<group by的字段, 编号>到PS_Expr的映射
                - 等每一条遍历完成之后，我们再遍历这个<group by的字段>的key来进行数据的计算？
    - 模糊查询 √
        - like好像也不难
        - 支持_ ? % [] 可以嘛
        - 用记忆化搜索来做，注意效率的问题
    - 嵌套查询 √
        - 怎么做？临时放到一个记录文件里？好像可以
        - from里的嵌套 
        - in的嵌套 
        - comp 一个数字的嵌套 
    - 重构代码 ×
        - 这代码写的也太脏了_(:з」∠)_
 - PS
    - 命令解析 √
 - 其他
    - 修改比较器的获取方式 √
    - 在比较器中方便地加入范围查找 × (不打算做)
    - 支持字符串的like表达式 √
    - 修改项目结构 ×
    - 可能存在的内存泄漏问题（lex） ×
    - 一些输出信息的补全 √
        - 运行时间
        - 成功更新/删除的条数
    - Float存储的时候要保留一定小数位    
    - 多表连接的时候列名的问题
    - 读Varchar怎么和便秘一样，根本不出结果 √
    - 这样的缺陷是varchar的大长度直接效果没了 √
    - group by的时候这一列值是null怎么办
 - 问题
    - mysql copy语法的实现？没搜到Orz
## TIPS
 - 序列化的是Varchar，但是内存中可以存成String
 - char类型存进去的时候后面多拷贝个\0方便格式化，所以实际上char(10)占用11个byte
 - RM_FileScan是否可以去除比较器，挪到Table来做？
 - insert统统用字符串来解析
 - 日期类型必须是yyyy-mm-dd格式的
 - 去掉numeric，全部为decimal，或者decimal(a,b)
 - AttrValue中的char*只做传递用，真正序列化和反序列化的时候不能用这儿的值
 - index为什么要用indexNo命名呢，直接用name不就可以了？（待改进）
 - 某些返回值是RC的没有TRY
 - 还需要支持BIGINT？
 - Expr里面的字符串都用std::string来表示
 - 需要一个得到行数的函数
 - 列的话不需要一开始就知道type
## Reference
 - https://github.com/huzecong/rebaseDB/tree/initial (Initial Redbase)
 - https://github.com/yifeih/redbase
 - https://github.com/parachvte/B-Plus-Tree
 - https://github.com/duzx16/MyDB
 
