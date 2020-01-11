use database eval;
select * from lineitem where L_LINENUMBER == 1;
alter table lineitem add index test(L_LINENUMBER);
select * from lineitem where L_LINENUMBER == 1;
alter table lineitem drop index test;
desc table lineitem;

select * from lineitem where L_SHIPDATE == '1996-03-13';
alter table lineitem add index test(L_SHIPDATE);
select * from lineitem where L_SHIPDATE == '1996-03-13';
alter table lineitem drop index test;
desc table lineitem;