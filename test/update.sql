create database test;
use database test;
drop table test_update;
create table test_update(  O_ORDERKEY      INT PRIMARY KEY,
                                O_CUSTKEY		INT NOT NULL,
                                O_ORDERSTATUS	CHAR(1),
                                O_TOTALPRICE	DECIMAL,
                                O_ORDERDATE		DATE,
                                O_ORDERPRIORITY	CHAR(15),
                                O_CLERK			CHAR(15),
                                O_SHIPPRIORITY	INT,
                                O_COMMENT		VARCHAR(79));
insert into test_update from '../../dataset/orders.tbl';
desc table test_update;
update test_update set O_CUSTKEY = 1 where O_ORDERSTATUS == 'O' and O_ORDERDATE > '1996-01-01';
update test_update set O_CUSTKEY = 2 where O_CUSTKEY == 1;
update test_update set O_CUSTKEY = 2 + O_ORDERKEY where O_CUSTKEY == 2;
update test_update set O_ORDERSTATUS = O_ORDERSTATUS + 'P' where O_CUSTKEY < 5;
update test_update set O_TOTALPRICE = O_TOTALPRICE * 10 where O_CUSTKEY < 5;
update test_update set O_ORDERDATE = '2019-12-8' where O_CUSTKEY < 5;
update test_update set O_ORDERDATE = '2019-12-28' where O_CUSTKEY < 5;
update test_update set O_ORDERDATE = null where O_CUSTKEY < 5;
update test_update set O_CUSTKEY = 100 where O_ORDERDATE is not null;
desc table test_update;
