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
delete from test_delete where O_ORDERKEY - 10 == 7000 + 2;
count table test_delete;
delete from test_delete where O_ORDERKEY == 7 * 1000+ 8 + 3;
count table test_delete;
delete from test_delete where 3 + 3 * 5 / 3 + 10%10 + O_ORDERKEY - 2 * 4 + 8 - 8== 6 + (3 + 1*2 + 1) * 1000 + 994+2*7;
count table test_delete;
delete from test_delete where (1 + O_ORDERKEY) % (7000 + 4 * 4) == 0;
count table test_delete;
delete from test_delete where O_ORDERKEY % 5 == 0;
count table test_delete;
delete from test_delete where O_ORDERKEY - 7050 < -1;
count table test_delete;
delete from test_delete where O_ORDERDATE < '1993-01-1';
count table test_delete;
delete from test_delete where O_ORDERDATE < '1993-01-01' + 1;
count table test_delete;
delete from test_delete where O_ORDERDATE < '1993-01-01';
count table test_delete;
delete from test_delete where O_ORDERKEY == 7000 + 73;
count table test_delete;
insert into test_delete from '../../dataset/orders_null.tbl';
count table test_delete;
delete from test_delete where O_ORDERSTATUS is not null;
count table test_delete;
delete from test_delete where O_ORDERSTATUS is null;
count table test_delete;
desc table test_delete;
insert into test_delete from '../../dataset/orders.tbl';
alter table test_delete drop primary key;
insert into test_delete from '../../dataset/orders.tbl';
insert into test_delete from '../../dataset/orders.tbl';
insert into test_delete from '../../dataset/orders.tbl';
insert into test_delete from '../../dataset/orders.tbl';
insert into test_delete from '../../dataset/orders.tbl';
insert into test_delete from '../../dataset/orders.tbl';
insert into test_delete from '../../dataset/orders.tbl';
insert into test_delete from '../../dataset/orders.tbl';
insert into test_delete from '../../dataset/orders.tbl';
delete from test_delete where O_CUSTKEY < 2;
alter table test_delete add index pri(O_CUSTKEY);
delete from test_delete where O_CUSTKEY < 4;
desc table test_delete;
delete from test_delete;
