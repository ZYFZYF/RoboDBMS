create database test;
use database test;
drop table constrain_primary_key;
create table constrain_primary_key(a int primary key, b int);
insert into constrain_primary_key values(1,1);
insert into constrain_primary_key values(2,2);


drop table constrain_foreign_key;
create table constrain_foreign_key(c int primary key, d int);
insert into constrain_foreign_key values(3,3);
alter table constrain_foreign_key add constraint test foreign key (d) references constrain_primary_key(a);//此处应该报错误无法建立外键
delete from constrain_foreign_key where d==3;
alter table constrain_foreign_key add constraint test foreign key (d) references constrain_primary_key(a);//此处建立成功
insert into constrain_foreign_key values(2,2);
insert into constrain_foreign_key values(4,4);//此处应该报错无法加入外键

delete from constrain_primary_key where a == 2;//此处应该报错有外键链着无法删除
delete from constrain_foreign_key where d == 2;
delete from constrain_primary_key where a == 2;//此处删除成功

alter table constrain_primary_key drop primary key;//此处应该报错有外键链着主键无法删除
alter table constrain_foreign_key drop foreign key test;
alter table constrain_primary_key drop primary key;//此处删除成功

desc table constrain_primary_key;
desc table constrain_foreign_key;

