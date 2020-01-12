use database eval;
create table nationBack(n_nationkey int not null,
                        n_name char(25) not null,
                        n_regionkey int not null,
                        n_comment varchar(152));
show tables;
desc table nationBack;
drop table nationBack;
show tables;
desc table nation;