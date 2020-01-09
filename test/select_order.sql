create database test;
use database test;

drop table test_order;
create table test_order(  O_ORDERKEY      INT PRIMARY KEY,
                                O_CUSTKEY		INT NOT NULL,
                                O_ORDERSTATUS	CHAR(1),
                                O_TOTALPRICE	DECIMAL,
                                O_ORDERDATE		DATE,
                                O_ORDERPRIORITY	CHAR(15),
                                O_CLERK			CHAR(15),
                                O_SHIPPRIORITY	INT,
                                O_COMMENT		VARCHAR(79));
insert into test_order from '../../dataset/orders.tbl';
select * from test_order order by O_CUSTKEY;
select * from test_order order by O_CUSTKEY limit 1000;
select * from test_order order by O_ORDERKEY,O_CUSTKEY limit 1000;
select * from test_order order by O_ORDERKEY,O_CUSTKEY DESC limit 1000;
select * from test_order order by O_ORDERSTATUS,O_ORDERKEY DESC limit 1000;
select * from test_order order by O_ORDERSTATUS,O_ORDERKEY DESC limit 1000,1000;
select * from test_order order by O_ORDERSTATUS,O_ORDERKEY DESC limit 1000,-1;