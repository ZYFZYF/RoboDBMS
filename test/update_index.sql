create database test;
use database test;
drop table update_index;
create table update_index(a char(10) primary key, b decimal(10,1), c varchar(12), d int);
alter table update_index add index test_1(b);
alter table update_index add index test_2(a);
alter table update_index add index test_2(d);
alter table update_index add index test_3(a,c);
desc table update_index;
alter table update_index drop index test_2;
alter table update_index add index test_4(b,d);
alter table update_index add index test_5(a,b,c,d);
alter table update_index drop index test_1;
alter table update_index drop index test_1;
alter table update_index drop index test_666;
desc table update_index;


