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
delete from test_delete where O_ORDERKEY < 100;
count table test_delete;
delete from test_delete where O_ORDERKEY < 200 and O_ORDERSTATUS == 'O';
count table test_delete;
delete from test_delete where O_ORDERKEY < 200 and O_ORDERSTATUS == 'F';
count table test_delete;
delete from test_delete where O_ORDERKEY < 200 and O_ORDERSTATUS == 'P';
count table test_delete;
delete from test_delete where O_ORDERKEY < 200 or not(O_ORDERKEY > 7000);
count table test_delete;
delete from test_delete where O_ORDERKEY == 7000 + 8 + 2;
count table test_delete;
