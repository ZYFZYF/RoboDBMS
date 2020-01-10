create database test;
use database test;

drop table test_like;
create table test_like(  O_ORDERKEY      INT PRIMARY KEY,
                                O_CUSTKEY		INT NOT NULL,
                                O_ORDERSTATUS	CHAR(1),
                                O_TOTALPRICE	DECIMAL,
                                O_ORDERDATE		DATE,
                                O_ORDERPRIORITY	CHAR(15),
                                O_CLERK			CHAR(15),
                                O_SHIPPRIORITY	INT,
                                O_COMMENT		VARCHAR(79));
insert into test_like from '../../dataset/orders.tbl';
select * from test_like where O_CLERK like '%1';
select * from test_like where O_CLERK like '%[13]1';
select * from test_like where O_CLERK like '%[^13]1';
select * from test_like where O_CLERK like ('%1', '%2');
select * from test_like where O_CLERK like ('%1', '%3_');
select * from test_like where O_CLERK like any('%1', '%2');


