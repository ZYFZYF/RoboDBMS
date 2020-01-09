create database test;
use database test;

drop table test_orders;
create table test_orders(  O_ORDERKEY      INT PRIMARY KEY,
                                O_CUSTKEY		INT NOT NULL,
                                O_ORDERSTATUS	CHAR(1),
                                O_TOTALPRICE	DECIMAL,
                                O_ORDERDATE		DATE,
                                O_ORDERPRIORITY	CHAR(15),
                                O_CLERK			CHAR(15),
                                O_SHIPPRIORITY	INT,
                                O_COMMENT		VARCHAR(79));
insert into test_orders from '../../dataset/orders.tbl';
select * from test_orders where O_ORDERSTATUS in ('O', 'F');
select * from test_orders where O_ORDERSTATUS not in ('O', 'P');
select * from test_orders where O_CUSTKEY in ('1', 1, 188, 'O', '20202020', 0999.000);
select * from test_orders where O_CUSTKEY not in ('1', 1, 188, 'O', '20202020', 0999.000);
select O_ORDERSTATUS, count(*) as NUM from test_orders group by O_ORDERSTATUS;
select * from test_orders where O_CUSTKEY in (select O_CUSTKEY from test_orders where O_CUSTKEY < 100);
select * from test_orders where O_CUSTKEY in (149);
select * from test_orders where O_CUSTKEY in (select MAX(O_CUSTKEY) as CUSTKEY from test_orders);
select * from test_orders where O_CUSTKEY > (2,3,4) order by O_CUSTKEY;
select * from test_orders where O_CUSTKEY < (2,3,4) order by O_CUSTKEY;
select * from test_orders where O_CUSTKEY < ANY (2,3,4) order by O_CUSTKEY desc;
select * from test_orders where O_CUSTKEY > ANY (2,3,4) order by O_CUSTKEY;
select O_CUSTKEY, count(*) as num from test_orders group by O_CUSTKEY order by O_CUSTKEY;
select * from test_orders where O_CUSTKEY > (select AVG(O_CUSTKEY) as avg_value from test_orders) order by O_CUSTKEY;
select * from test_orders where O_CUSTKEY > 380 order by O_CUSTKEY;
select * from test_orders where O_CUSTKEY == ALL (select O_CUSTKEY from test_orders where O_CUSTKEY % 2 ==0) order by O_CUSTKEY;
select * from test_orders where O_CUSTKEY == ANY (select O_CUSTKEY from test_orders where O_CUSTKEY % 2 ==0) order by O_CUSTKEY;
select * from test_orders where O_CUSTKEY < ANY (select O_CUSTKEY from test_orders where O_CUSTKEY % 2 ==0) order by O_CUSTKEY;
select * from test_orders where O_CUSTKEY < ALL (select O_CUSTKEY from test_orders where O_CUSTKEY % 2 ==0) order by O_CUSTKEY;
select count(*) as num from test_orders where O_CUSTKEY % 3 == 0;





