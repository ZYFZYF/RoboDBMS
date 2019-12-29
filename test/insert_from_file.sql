create database test;
use database test;
drop table insert_from_file;
create table insert_from_file(  O_ORDERKEY      INT PRIMARY KEY,
                                O_CUSTKEY		INT NOT NULL,
                                O_ORDERSTATUS	CHAR(1),
                                O_TOTALPRICE	DECIMAL,
                                O_ORDERDATE		DATE,
                                O_ORDERPRIORITY	CHAR(15),
                                O_CLERK			CHAR(15),
                                O_SHIPPRIORITY	INT,
                                O_COMMENT		VARCHAR(79));
insert into insert_from_file from '../../dataset/orders.tbl';
desc table insert_from_file;
insert into insert_from_file from '../../dataset/orders_null.tbl';
desc table insert_from_file;
insert into insert_from_file values (7505,-1800,null,-11.2212,,,,,);
insert into insert_from_file values (7506,null,,,,,,,);
insert into insert_from_file values (7507,-1800,null,-11.2212,,,,,);
insert into insert_from_file values (7508,-1800,,NULL,,,,,);
desc table insert_from_file;
show table insert_from_file;