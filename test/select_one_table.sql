create database test;
use database test;
drop table select_one_table;
create table select_one_table(  O_ORDERKEY      INT PRIMARY KEY,
                                O_CUSTKEY		INT NOT NULL,
                                O_ORDERSTATUS	CHAR(1),
                                O_TOTALPRICE	DECIMAL,
                                O_ORDERDATE		DATE,
                                O_ORDERPRIORITY	CHAR(15),
                                O_CLERK			CHAR(15),
                                O_SHIPPRIORITY	INT,
                                O_COMMENT		VARCHAR(79));
insert into select_one_table from '../../dataset/orders.tbl';
desc table select_one_table;
select O_ORDERDATE from select_one_table;
select O_CUSTKEY from select_one_table;
select O_ORDERKEY from select_one_table;
select O_ORDERKEY, O_CUSTKEY from select_one_table;
select O_ORDERKEY, O_CUSTKEY as TAT from select_one_table;
select O_ORDERKEY from select_one_table where O_ORDERSTATUS == 'P';
select O_ORDERKEY from select_one_table where O_ORDERKEY % 10 == 0;
select O_ORDERKEY from select_one_table where O_ORDERKEY % 10 == 0 or O_ORDERSTATUS == 'P';
select * from select_one_table;
select * from select_one_table where O_ORDERKEY % 10 == 0 or O_ORDERSTATUS == 'P' or O_CUSTKEY % 10 == 5 ;