create database test;
use database test;
drop table test_delete;
create table test_delete(  O_ORDERKEY      INT PRIMARY KEY,
                                O_CUSTKEY		INT NOT NULL,
                                O_ORDERSTATUS	CHAR(1),
                                O_TOTALPRICE	DECIMAL,
                                O_ORDERDATE		DATE,
                                O_ORDERPRIORITY	CHAR(15),
                                O_CLERK			CHAR(15),
                                O_SHIPPRIORITY	INT,
                                O_COMMENT		VARCHAR(79));
insert into test_delete from '../../dataset/orders.tbl';
count table test_delete;
delete from test_delete;
count table test_delete;
insert into test_delete from '../../dataset/orders.tbl';
count table test_delete;