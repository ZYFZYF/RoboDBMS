create database test;
use database test;

drop table test_aggregation;
create table test_aggregation(  O_ORDERKEY      INT PRIMARY KEY,
                                O_CUSTKEY		INT NOT NULL,
                                O_ORDERSTATUS	CHAR(1),
                                O_TOTALPRICE	DECIMAL,
                                O_ORDERDATE		DATE,
                                O_ORDERPRIORITY	CHAR(15),
                                O_CLERK			CHAR(15),
                                O_SHIPPRIORITY	INT,
                                O_COMMENT		VARCHAR(79));
insert into test_aggregation from '../../dataset/orders.tbl';

select 2 * max(O_CUSTKEY + 1) + 1 * 3 as MAX_VALUE_PLUS from test_aggregation;
select avg(O_CUSTKEY) as AVG_VALUE_1 from test_aggregation;
select max(O_CUSTKEY) as MAX_VALUE from test_aggregation;
select max(O_CUSTKEY) as MAX_VALUE, min(O_CUSTKEY) as MIN_VALUE from test_aggregation;
select max(O_CUSTKEY) as MAX_VALUE, min(O_CUSTKEY) as MIN_VALUE, avg(O_CUSTKEY) as AVG_VALUE_1 from test_aggregation;
select max(O_CUSTKEY) as MAX_VALUE, min(O_CUSTKEY) as MIN_VALUE, avg(O_CUSTKEY) as AVG_VALUE_1, count(O_CUSTKEY) as COUNT_VALUE from test_aggregation;
select max(O_CUSTKEY) as MAX_VALUE, min(O_CUSTKEY) as MIN_VALUE, avg(O_CUSTKEY) as AVG_VALUE_1, count(O_CUSTKEY) as COUNT_VALUE, sum(O_CUSTKEY) as SUM_VALUE from test_aggregation;
select max(O_CUSTKEY) as MAX_VALUE, min(O_CUSTKEY) as MIN_VALUE, avg(O_CUSTKEY) as AVG_VALUE_1, count(O_CUSTKEY) as COUNT_VALUE, sum(O_CUSTKEY) as SUM_VALUE, sum(O_CUSTKEY) / count(O_CUSTKEY) as AVG_VALUE_2 from test_aggregation;
select max(O_CLERK) as MAX_O_CLERK, min(O_CLERK) as MIN_O_CLERK from test_aggregation;
select max(O_ORDERDATE) as MAX_O_DATE, min(O_ORDERDATE) as MIN_O_DATE from test_aggregation;