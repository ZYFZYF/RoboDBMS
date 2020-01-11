create database test;
use database test;
drop table primary_key;
create table primary_key(a int primary key, b int);
insert into primary_key values(1,1);
insert into primary_key values(2,2);


drop table foreign_key;
create table foreign_key(c int primary key, d int);
insert into foreign_key values(3,1);
insert into foreign_key values(4,2);
alter table foreign_key add constraint test foreign key (d) references primary_key(a);

alter table primary_key add f int not null default 100;
alter table primary_key add g date default '2019-12-31';
alter table primary_key add h int;
alter table primary_key add i int not null;
desc table primary_key;
