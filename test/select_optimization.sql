create database test;
use database test;

drop table test_order;
create table test_order(  O_ORDERKEY      INT,
                                O_CUSTKEY		INT NOT NULL,
                                O_ORDERSTATUS	CHAR(1),
                                O_TOTALPRICE	DECIMAL,
                                O_ORDERDATE		DATE,
                                O_ORDERPRIORITY	CHAR(15),
                                O_CLERK			CHAR(15),
                                O_SHIPPRIORITY	INT,
                                O_COMMENT		VARCHAR(79));
insert into test_order from '../../dataset/orders.tbl';
insert into test_order from '../../dataset/orders.tbl';
insert into test_order from '../../dataset/orders.tbl';
insert into test_order from '../../dataset/orders.tbl';
insert into test_order from '../../dataset/orders.tbl';
insert into test_order from '../../dataset/orders.tbl';
insert into test_order from '../../dataset/orders.tbl';
insert into test_order from '../../dataset/orders.tbl';
insert into test_order from '../../dataset/orders.tbl';
insert into test_order from '../../dataset/orders.tbl';


drop table test_customer;
create table test_customer (
		C_CUSTKEY		INT PRIMARY KEY,
		C_NAME			VARCHAR(25),
		C_ADDRESS		VARCHAR(40),
		C_NATIONKEY		INT NOT NULL,
		C_PHONE			CHAR(15),
		C_ACCTBAL		DECIMAL,
		C_MKTSEGMENT	CHAR(10),
		C_COMMENT		VARCHAR(117)
	);
insert into test_customer from '../../dataset/customer.tbl';
select * from test_order, test_customer where test_order.O_CUSTKEY == test_customer.C_CUSTKEY and test_order.O_ORDERSTATUS == 'P';
alter table test_order add index statusindex(O_ORDERSTATUS);
select * from test_order, test_customer where test_order.O_CUSTKEY == test_customer.C_CUSTKEY and test_order.O_ORDERSTATUS == 'P';
select * from test_order, test_customer where test_order.O_CUSTKEY == test_customer.C_CUSTKEY and test_order.O_ORDERDATE == '1997-06-23';
alter table test_order add index dateindex(O_ORDERDATE);
select * from test_order, test_customer where test_order.O_CUSTKEY == test_customer.C_CUSTKEY and test_order.O_ORDERDATE == '1997-06-23';
select * from test_order, test_customer where test_order.O_CUSTKEY == test_customer.C_CUSTKEY and test_order.O_CUSTKEY == 1;
alter table test_order add index custkeyindex(O_CUSTKEY);
select * from test_order, test_customer where test_order.O_CUSTKEY == test_customer.C_CUSTKEY and test_order.O_CUSTKEY == 1;
select * from test_order, test_customer where test_order.O_CUSTKEY == test_customer.C_CUSTKEY and test_order.O_CUSTKEY < 100;
select * from test_order, test_customer where test_order.O_CUSTKEY == test_customer.C_CUSTKEY and test_order.O_ORDERSTATUS == 'P';