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
select * from test_update order by O_CUSTKEY;
update test_update set O_CUSTKEY = 1 where O_ORDERSTATUS == 'O' and O_ORDERDATE > '1996-01-01';
update test_update set O_CUSTKEY = 2 where O_CUSTKEY == 1;
update test_update set O_CUSTKEY = 2 + O_ORDERKEY where O_CUSTKEY == 2;
update test_update set O_ORDERSTATUS = O_ORDERSTATUS + 'P' where O_CUSTKEY < 5;
update test_update set O_TOTALPRICE = O_TOTALPRICE * 10 where O_CUSTKEY < 5;
update test_update set O_ORDERDATE = '2019-12-8' where O_CUSTKEY < 5;
update test_update set O_ORDERDATE = '2019-12-28' where O_CUSTKEY < 5;
update test_update set O_ORDERDATE = null where O_CUSTKEY < 5;
update test_update set O_CUSTKEY = 100 where O_ORDERDATE is not null;
update test_update set O_ORDERPRIORITY = O_ORDERPRIORITY + 'TEST' where O_ORDERDATE is not null;
update test_update set O_TOTALPRICE = O_ORDERKEY + O_CUSTKEY where O_ORDERDATE is not null;
desc table test_update;
alter table test_update drop primary key;
insert into test_update from '../../dataset/orders.tbl';
insert into test_update from '../../dataset/orders.tbl';
insert into test_update from '../../dataset/orders.tbl';
insert into test_update from '../../dataset/orders.tbl';
insert into test_update from '../../dataset/orders.tbl';
insert into test_update from '../../dataset/orders.tbl';
insert into test_update from '../../dataset/orders.tbl';
insert into test_update from '../../dataset/orders.tbl';
insert into test_update from '../../dataset/orders.tbl';
insert into test_update from '../../dataset/orders.tbl';
update test_update set O_TOTALPRICE = 1000 where O_CUSTKEY == 3;
desc table test_update;
alter table test_update add index pri(O_CUSTKEY);
update test_update set O_TOTALPRICE = 10000 where O_CUSTKEY == 3;
desc table test_update;
update test_update set O_TOTALPRICE = 1000000 where O_CUSTKEY == 4;
desc table test_update;
