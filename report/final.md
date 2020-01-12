#《数据库系统概论》大作业项目总结

<center> 计65 赵鋆峰 2016011373 </center>
## 实验结果

最终，我实现了一个支持丰富语法的单机版数据库，具体实现的功能有：

- 支持数据类型
	- int
	- decimal：包括decimal（x,y）其中x代表总有效数字位数，y代表小数位数
	- char：最大255
	- varchar：没有上限
	- date：严格的yyyy-mm-dd格式
- 支持条件表达式
	- 支持运算
		- 比较运算：=、 !=、 <、 <=、 >、 >=、 in、 not in、like
			- 其中like支持\%、_、?、[]、[^]
		- 逻辑运算：and、or、not
		- 算术运算：+、-、\*、/、%
		- 其他运算
			- 支持小括号
			- 支持any、all语法，例如 a == any(1,2,3)代表a只要是1、2、3中的任意一个就为真，而a != all(1,2,3)代表a必须不是1、2、3中的任意一个才为真
	- 参与运算的元素
		- 常量
		- 常量数组，例如 where a in (1,2,3)
		- 列，例如3 + 3 * 5 / 3 + 10%10 + O_ORDERKEY - 2 * 4 + 8 - 8== 6 + (3 + 1*2 + 1) * 1000 + 994+2*7
		- 查询结果，例如 where a > (select avg(a) as avg\_value from b)
- 支持语法
	- DDL
		- 数据库相关：创建、删除、使用、显示数据库
		- 表相关：创建、删除、更名
		- 列相关：增加、删除、更名、更新（增加+删除）
		- 完整性约束相关：主键增删、外键增删
		- 索引相关
			- 增加
			- 删除
			- 支持所有数据类型的索引包括联合索引，并且联合索引也可以是不同的类型
	- DML
		- 增
			- 单行插入（可选插入列，其他列认为是NULL）
			- 文件导入
			- 完整性检查
		- 删
			- 复杂条件表达式
			- 完整性检查
			- 用索引优化遍历
				- 单列索引支持=、!=、<、<=、>、>=
				- 多列索引只支持对=检索
		- 改
			- 复杂条件表达式
			- 完整性检查
			- 索引优化同删除
		- 查
			- 支持\*查询所有列
			- 多表连接（不限数目）
				- 查询优化
					- 对表的顺序用预估函数进行调整
					- 索引优化遍历
			- 嵌套查询
				- 多列结果可以当做from的表项
				- 单列结果可以作为条件表达式参与比较
			- 分组查询
				- 简单聚集函数max、min、avg、sum、count
				- 复杂聚集：可以对聚集之前/之后的结果进行运算：例如min(age+1)、min(age) + 1、max(age) + min(age)
			- 排序
				- 单列、多列：例如select * from test order by a,b
				- 升序、降序：例如select * from test order by a desc
				- limit选取条数，支持指定开始选取的offset：例如select a,min(b) as min\_b from test group by a order by min_b desc limit 1,5意为从分组中b最小值最大的1-6位输出


除此之外还提供了：

 - 丰富的输出信息帮助调试和结果查看
 - 丰富的测例

## 系统架构设计

## 各模块详细设计

## 代码说明

## 主要接口说明

## 小组分工

全部工作为赵鋆峰一人完成，除去提供的文件系统后总代码量如下：

![code](code.png)

## 项目地址

全部代码以及提交记录存放在Github上：https://github.com/ZYFZYF/RoboDBMS

## 参考文献

参考了Github上斯坦福课程作业、基于内存的B+树以及往年同学作业的一些思想

 - https://github.com/huzecong/rebaseDB/tree/initial (Initial Redbase)
 - https://github.com/yifeih/redbase
 - https://github.com/parachvte/B-Plus-Tree
 - https://github.com/duzx16/MyDB

