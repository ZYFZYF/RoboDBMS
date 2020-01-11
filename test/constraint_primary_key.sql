create database test;
use database test;
drop table constrain_primary_key;
create table constrain_primary_key(a int primary key, b int);
insert into constrain_primary_key values(1,1);
insert into constrain_primary_key values(2,2);


drop table constrain_foreign_key;
create table constrain_foreign_key(c int primary key, d int);
insert into constrain_foreign_key values(3,3);
alter table constrain_foreign_key add constraint test foreign key (d) references constrain_primary_key(a);
insert into constrain_foreign_key values(4,4);
desc table constrain_primary_key;
desc table constrain_foreign_key;

