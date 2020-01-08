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
select O_ORDERSTATUS, MIN(O_CUSTKEY) as MIN_CUSTKEY, MAX(O_CUSTKEY) as MAX_CUSTKEY, AVG(O_CUSTKEY) as AVG_CUSTKEY, COUNT(*) as COUNT_CUSTKEY from test_group group by O_ORDERSTATUS;
select O_ORDERSTATUS, MIN(O_CUSTKEY) as MIN_CUSTKEY, MAX(O_CUSTKEY) as MAX_CUSTKEY, AVG(O_CUSTKEY) as AVG_CUSTKEY, COUNT(O_ORDERSTATUS)  as COUNT_CUSTKEY from test_group group by O_ORDERSTATUS;
select COUNT(*) as NUM, O_ORDERDATE from test_group group by O_ORDERDATE;
