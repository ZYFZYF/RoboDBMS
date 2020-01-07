create database test;
use database test;

drop table test_group;
create table test_group(  O_ORDERKEY      INT PRIMARY KEY,
                                O_CUSTKEY		INT NOT NULL,
                                O_ORDERSTATUS	CHAR(1),
                                O_TOTALPRICE	DECIMAL,
                                O_ORDERDATE		DATE,
                                O_ORDERPRIORITY	CHAR(15),
                                O_CLERK			CHAR(15),
                                O_SHIPPRIORITY	INT,
                                O_COMMENT		VARCHAR(79));
insert into test_group from '../../dataset/orders.tbl';

select max(O_CUSTKEY+1) + 1 as MAX_VALUE_PLUS from test_group;
select avg(O_CUSTKEY) as AVG_VALUE_1 from test_group;
select max(O_CUSTKEY) as MAX_VALUE from test_group;
select max(O_CUSTKEY) as MAX_VALUE, min(O_CUSTKEY) as MIN_VALUE from test_group;
select max(O_CUSTKEY) as MAX_VALUE, min(O_CUSTKEY) as MIN_VALUE, avg(O_CUSTKEY) as AVG_VALUE_1 from test_group;
select max(O_CUSTKEY) as MAX_VALUE, min(O_CUSTKEY) as MIN_VALUE, avg(O_CUSTKEY) as AVG_VALUE_1, count(O_CUSTKEY) as COUNT_VALUE from test_group;
select max(O_CUSTKEY) as MAX_VALUE, min(O_CUSTKEY) as MIN_VALUE, avg(O_CUSTKEY) as AVG_VALUE_1, count(O_CUSTKEY) as COUNT_VALUE, sum(O_CUSTKEY) as SUM_VALUE from test_group;
select max(O_CUSTKEY) as MAX_VALUE, min(O_CUSTKEY) as MIN_VALUE, avg(O_CUSTKEY) as AVG_VALUE_1, count(O_CUSTKEY) as COUNT_VALUE, sum(O_CUSTKEY) as SUM_VALUE, sum(O_CUSTKEY) / count(O_CUSTKEY) as AVG_VALUE_2 from test_group;
select max(O_CLERK) as MAX_O_CLERK, min(O_CLERK) as MIN_O_CLERK from test_group;