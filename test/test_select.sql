use database eval;
select * from partsupp,part,supplier where PS_PARTKEY == P_PARTKEY and S_SUPPKEY == PS_SUPPKEY;
select * from part,partsupp,supplier where PS_PARTKEY == P_PARTKEY and S_SUPPKEY == PS_SUPPKEY;
select * from supplier,partsupp,part where PS_PARTKEY == P_PARTKEY and S_SUPPKEY == PS_SUPPKEY;
select * from supplier,part,partsupp where PS_PARTKEY == P_PARTKEY and S_SUPPKEY == PS_SUPPKEY;
select * from lineitem,partsupp,orders where L_PARTKEY == PS_PARTKEY and L_SUPPKEY == PS_SUPPKEY and L_ORDERKEY == O_ORDERKEY;
select * from partsupp,lineitem,orders where L_PARTKEY == PS_PARTKEY and L_SUPPKEY == PS_SUPPKEY and L_ORDERKEY == O_ORDERKEY;
select * from orders,partsupp,lineitem where L_PARTKEY == PS_PARTKEY and L_SUPPKEY == PS_SUPPKEY and L_ORDERKEY == O_ORDERKEY;