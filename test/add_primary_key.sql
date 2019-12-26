create database test;
use database test;
drop table add_primary_key;
create table add_primary_key(a char(10) primary key, b decimal(10,1));
insert into add_primary_key values ('aaaa', 10.12);
insert into add_primary_key values ('aaaa', 9);
insert into add_primary_key values ('aaaaa', 10.34);
insert into add_primary_key values ('bbbb', 1000000000001.34);
insert into add_primary_key values ('bbbbc', 10);
desc table add_primary_key;

drop table add_primary_key;
create table add_primary_key(a char(10), b decimal(10,1));
insert into add_primary_key values ('aaaa', 10.12);
insert into add_primary_key values ('aaaa', 9);
desc table add_primary_key;

drop table add_primary_key;
create table add_primary_key(a char(10), b decimal(10,1));
insert into add_primary_key values ('aaaa', 10.12);
alter table add_primary_key add primary key(a);
insert into add_primary_key values ('aaaa', 9);
alter table add_primary_key drop primary key;
insert into add_primary_key values ('aaaa', 9);
alter table add_primary_key add primary key(a);
desc table add_primary_key;

drop table add_primary_key;
create table add_primary_key(a char(10), b int);
insert into add_primary_key values ('aaaa', 100);
alter table add_primary_key add primary key(a,b);
insert into add_primary_key values ('aaaa', 100);
insert into add_primary_key values ('aaaa', 88);
insert into add_primary_key values ('bbbb', 100);
alter table add_primary_key drop primary key;
insert into add_primary_key values ('aaaa', 100);
alter table add_primary_key add primary key(a,b);
desc table add_primary_key;





