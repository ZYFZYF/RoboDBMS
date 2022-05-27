### 单线程计划执行
 - 处理模型
   - Iterator Model（又叫Volcano或者Pipeline Model）、Materialization Model、Vectorized / Batch Model
   - 把select语句组织成关系代数的树的模式，然后每个节点有一个next()函数
   - 三者分别是一次返回一个、返回整个和返回一个batch
   - 优劣：
     - 后两者适合OLAP场景，因为数据量不多放得下，而且函数调用少
     - 前者的输出控制很好做
   - RoboDBMS的实现方式有点类似Iterator，但是是一把梭的样子...没有形成树的样子
     - where是Iterator，但是order group by之类的是Materialization
     - 主要是因为RoboDBMS的SQL语言和传统的不一样，join混在where里，而且支持任意奇怪的跨表比较，所以不是很好用关系代数表示，也就不好表示成那种树的结构，因此优化也难一些
   - 我其实是维护了目前每个表选取的tuple的信息然后去遍历的，可以理解为不用next()进行传递？而在嵌套子查询之类的，因为是基于bison的，所以我只能是把结果全部算出来（不过语法树的话好像也得这样）
 - 访问方式
   - Sequence Scan
     - 优化方法
       - Zone Map：对一个page建立一些统计值，快速筛选
   - Index Scan

### 多线程计划执行
 - inter-query vs intra-query
 - intra-query
   - intra-operator(horizontal): 数据分段
   - inter-operator(vertical): 不影响的operator可以并行执行
   - busy parallelism: 混用
   - 但并行的话可能对硬盘不友好
 - I/O并行
   - 数据存在不同的硬盘上
   - 数据分片
     - 垂直分片：按字段分在不同的位置
     - 水平分片：一批数据胖仔一个位置（hash、range、predicate）