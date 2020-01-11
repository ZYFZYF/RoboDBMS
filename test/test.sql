create database eval;
use database eval;
drop table part;
create table part (
		P_PARTKEY		INT PRIMARY KEY,
		P_NAME			VARCHAR(55),
		P_MFGR			CHAR(25),
		P_BRAND			CHAR(10),
		P_TYPE			VARCHAR(25),
		P_SIZE			INT,
		P_CONTAINER		CHAR(10),
		P_RETAILPRICE	DECIMAL,
		P_COMMENT		VARCHAR(23)
	);
	
drop table region;
create table region (
		R_REGIONKEY	INT PRIMARY KEY,
		R_NAME		CHAR(25),
		R_COMMENT	VARCHAR(152)
	);

drop table nation;
create table nation (
		N_NATIONKEY		INT PRIMARY KEY,
		N_NAME			CHAR(25),
		N_REGIONKEY		BIGINT NOT NULL,
		N_COMMENT		VARCHAR(152)
	);

drop table supplier;
create table supplier (
		S_SUPPKEY		INT PRIMARY KEY,
		S_NAME			CHAR(25),
		S_addRESS		VARCHAR(40),
		S_NATIONKEY		BIGINT NOT NULL,
		S_PHONE			CHAR(15),
		S_ACCTBAL		DECIMAL,
		S_COMMENT		VARCHAR(101)
	);

drop table customer;
create table customer (
		C_CUSTKEY		INT PRIMARY KEY,
		C_NAME			VARCHAR(25),
		C_addRESS		VARCHAR(40),
		C_NATIONKEY		BIGINT NOT NULL,
		C_PHONE			CHAR(15),
		C_ACCTBAL		DECIMAL,
		C_MKTSEGMENT	CHAR(10),
		C_COMMENT		VARCHAR(117)
	);

drop table partsupp;
create table partsupp (
		PS_PARTKEY		BIGINT NOT NULL,
		PS_SUPPKEY		BIGINT NOT NULL,
		PS_AVAILQTY		INTEGER,
		PS_SUPPLYCOST	DECIMAL,
		PS_COMMENT		VARCHAR(199)
	);
alter table partsupp add primary key(PS_PARTKEY, PS_SUPPKEY);

drop table orders;
create table orders (
		O_ORDERKEY		INT PRIMARY KEY,
		O_CUSTKEY		BIGINT NOT NULL,
		O_ORDERSTATUS	CHAR(1),
		O_TOTALPRICE	DECIMAL,
		O_ORDERDATE		DATE,
		O_ORDERPRIORITY	CHAR(15),
		O_CLERK			CHAR(15),
		O_SHIPPRIORITY	INT,
		O_COMMENT		VARCHAR(79)
	);

drop table lineitem;
create table lineitem (
		L_ORDERKEY		BIGINT NOT NULL,
		L_PARTKEY		BIGINT NOT NULL,
		L_SUPPKEY		BIGINT NOT NULL,
		L_LINENUMBER	INT,
		L_QUANTITY		DECIMAL,
		L_EXTENDEDPRICE	DECIMAL,
		L_DISCOUNT		DECIMAL,
		L_TAX			DECIMAL,
		L_RETURNFLAG	CHAR(1),
		L_LINESTATUS	CHAR(1),
		L_SHIPDATE		DATE,
		L_COMMITDATE	DATE,
		L_RECEIPTDATE	DATE,
		L_SHIPINSTRUCT	CHAR(25),
		L_SHIPMODE		CHAR(10),
		L_COMMENT		VARCHAR(44)
	);
alter table lineitem add primary key(L_ORDERKEY, L_LINENUMBER);

insert into part from '../../dataset/part.tbl';
insert into region from '../../dataset/region.tbl';
insert into nation from '../../dataset/nation.tbl';
insert into supplier FROM '../../dataset/supplier.tbl';
insert into customer FROM '../../dataset/customer.tbl';
insert into partsupp FROM '../../dataset/partsupp.tbl';
insert into orders FROM '../../dataset/orders.tbl';
insert into lineitem FROM '../../dataset/lineitem.tbl';


alter table supplier add constraint test foreign key (S_NATIONKEY) references nation(N_NATIONKEY);
alter table partsupp add constraint test_1 foreign key (PS_PARTKEY) references part(P_PARTKEY);
alter table partsupp add constraint test_2 foreign key (PS_SUPPKEY) references supplier(S_SUPPKEY);
alter table customer add constraint test foreign key (C_NATIONKEY) references nation(N_NATIONKEY);
alter table orders add constraint test foreign key (O_CUSTKEY) references customer(C_CUSTKEY);
alter table lineitem add constraint test_1 foreign key (L_ORDERKEY) references orders(O_ORDERKEY);
alter table lineitem add constraint test_2 foreign key (L_PARTKEY,L_SUPPKEY) references partsupp(PS_PARTKEY,PS_SUPPKEY);
alter table nation add constraint test foreign key (N_REGIONKEY) references region(R_REGIONKEY);