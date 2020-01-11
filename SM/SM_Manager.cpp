//
// Created by 赵鋆峰 on 2019/12/2.
//

#include "SM_Manager.h"
#include "../utils/Utils.h"
#include "SM_Table.h"

SM_Manager::SM_Manager() : pfManager(PF_Manager::Instance()), rmManager(RM_Manager::Instance()),
                           ixManager(IX_Manager::Instance()), isUsingDb(false) {

    //Meta文件中存储了所有的数据库管理系统的信息
    dbmsMetaFile = Utils::getAbsolutePath("DBMS.Meta");
    //如果没有该文件，那么要初始化一个带meta的meta文件，否则从该文件中读取meta
    if (access(dbmsMetaFile.c_str(), F_OK) < 0) {
        pfManager.CreateFileWithMeta(dbmsMetaFile.c_str(), &dbmsMeta, sizeof(DbmsMeta));
    } else {
        pfManager.GetMeta(dbmsMetaFile.c_str(), &dbmsMeta, sizeof(DbmsMeta));
    }
}

SM_Manager::~SM_Manager() {

}

RC SM_Manager::OpenDb(const char *dbName) {
    if (chdir(dbName) < 0) {
        return SM_DB_NOT_EXIST;
    }
    //打开的时候读，修改的话立马写，不用等到关的时候再写
    ReadDbMeta();
    return OK_RC;
}

RC SM_Manager::CloseDb() {
    //以防万一关闭的时候还是再写一次
    WriteDbMeta();
    chdir("..");
    return OK_RC;
}

RC SM_Manager::DropTable(const char *tbName) {
    if (!isUsingDb) return SM_NOT_USING_DATABASE;

    TableId tableId = GetTableIdFromName(tbName);
    if (tableId < 0) return SM_TABLE_NOT_EXIST;

    if (dbMeta.tableMetas[tableId].primaryKey.referenceNum) return SM_TABLE_HAS_REFERENCE;
    //删掉记录文件
    rmManager.DestroyFile(Utils::getRecordFileName(dbMeta.tableMetas[tableId].createName).c_str());
    //删掉字符池文件
    SP_Manager::DestroyStringPool(Utils::getStringPoolFileName(dbMeta.tableMetas[tableId].createName).c_str());
    //删掉索引文件
    for (int i = 0; i < MAX_INDEX_NUM; i++)
        if (dbMeta.tableMetas[tableId].indexes[i].keyNum) {
            ixManager.DestroyIndex(dbMeta.tableMetas[tableId].createName, i);
        }
    memset(&dbMeta.tableMetas[tableId], 0, sizeof(TableMeta));
    memset(dbMeta.tableNames[tableId], 0, sizeof(dbMeta.tableNames[tableId]));

    WriteDbMeta();
    return OK_RC;
}

RC SM_Manager::Load(const char *relName, const char *fileName) {
    return PF_NOBUF;
}

RC SM_Manager::Help() {
    return PF_NOBUF;
}

RC SM_Manager::Help(const char *relName) {
    return PF_NOBUF;
}

RC SM_Manager::Print(const char *relName) {
    return PF_NOBUF;
}

RC SM_Manager::Set(const char *paramName, const char *value) {
    return PF_NOBUF;
}

RC SM_Manager::CreateDb(const char *dbName) {
    for (auto &db : dbmsMeta.databaseName)
        if (strcmp(db, dbName) == 0) {
            return SM_DB_ALREADY_IN;
        }
    for (auto &i : dbmsMeta.databaseName) {
        if (strlen(i) == 0) {
            strcpy(i, dbName);
            if (mkdir(dbName, S_IRWXU) < 0) {
                return SM_UNIX;
            }
            TRY(pfManager.UpdateMeta(dbmsMetaFile.c_str(), &dbmsMeta, sizeof(DbmsMeta)));
            return OK_RC;
        }
    }
    return SM_DB_IS_FULL;
}

RC SM_Manager::DropDb(const char *dbName) {
    for (auto &db : dbmsMeta.databaseName) {
        if (strcmp(db, dbName) == 0) {
            memset(db, 0, sizeof(db));
            if (rmdir(dbName) < 0) {
                return SM_UNIX;
            }
            TRY(pfManager.UpdateMeta(dbmsMetaFile.c_str(), &dbmsMeta, sizeof(DbmsMeta)));
            return OK_RC;
        }
    }
    return SM_DB_NOT_EXIST;
}

RC SM_Manager::UseDb(const char *dbName) {
    if (isUsingDb) {
        TRY(CloseDb());
    }
    TRY(OpenDb(dbName));
    isUsingDb = true;
    return OK_RC;
}

RC SM_Manager::AddColumn(const char *tbName, ColumnDesc columnDesc) {
    TableId tableId = GetTableIdFromName(tbName);
    if (tableId < 0)return SM_TABLE_NOT_EXIST;
    SM_Table table(tableId);
    std::string preName = dbMeta.tableMetas[tableId].createName;
    TRY(table.addColumn(columnDesc))
    std::string nowName = dbMeta.tableMetas[tableId].createName;
    //执行文件的alias操作
    //记录文件
    remove(Utils::getRecordFileName(preName.data()).data());
    rename(Utils::getRecordFileName(nowName.data()).data(), Utils::getRecordFileName(preName.data()).data());
    //字符池文件
    remove(Utils::getStringPoolFileName(preName.data()).data());
    rename(Utils::getStringPoolFileName(nowName.data()).data(), Utils::getStringPoolFileName(preName.data()).data());
    //索引文件
    for (int i = 0; i < MAX_INDEX_NUM; i++)
        if (dbMeta.tableMetas[tableId].indexes[i].keyNum) {
            remove(Utils::getIndexFileName(preName.data(), i).data());
            rename(Utils::getIndexFileName(nowName.data(), i).data(),
                   Utils::getIndexFileName(preName.data(), i).data());
        }
    strcpy(dbMeta.tableMetas[tableId].createName, preName.data());
    return OK_RC;
}

RC SM_Manager::DropColumn(const char *dbName, const char *columnName) {
    return PF_NOBUF;
}

SM_Manager &SM_Manager::Instance() {
    static SM_Manager instance;
    return instance;
}

RC SM_Manager::ShowDatabases() {
    int cnt = 0;
    for (auto &db : dbmsMeta.databaseName)
        if (strlen(db) > 0) {
            cnt++;
            printf("%30s", db);
            if (++cnt % 5 == 0) {
                printf("\n");
            }
        }
    printf("\n");
    return OK_RC;
}

RC SM_Manager::DescDatabase(const char &dbName) {
    return PF_EOF;
}

RC SM_Manager::ShowTables() {
    if (!isUsingDb) {
        return SM_NOT_USING_DATABASE;
    }
    int cnt = 0;
    for (auto &tb : dbMeta.tableNames)
        if (strlen(tb) > 0) {
            cnt++;
            printf("%30s", tb);
            if (++cnt % 5 == 0) {
                printf("\n");
            }
        }
    printf("\n");
    return OK_RC;
}

std::string splitLine = "---------------------------------------------------------------------------------------------------------------------";

RC SM_Manager::DescTable(const char *tbName) {
    if (!isUsingDb) {
        return SM_NOT_USING_DATABASE;
    }
    TableId tableId = GetTableIdFromName(tbName);
    if (tableId < 0)return SM_TABLE_NOT_EXIST;
    SM_Table table(tableId);
    //列内容展示
    printf("Column\n");
    char indent[] = "\t\t";
    printf("%s%s\n", indent, splitLine.data());
    printf("%s%-30s%-30s%-30s%-30s\n", indent, "name", "type", "nullable", "default");
    printf("%s%s\n", indent, splitLine.data());
    for (int i = 0; i < dbMeta.tableMetas[tableId].columnNum; i++) {
        ColumnDesc column = dbMeta.tableMetas[tableId].columns[i];
        if (strlen(column.name) > 0) {
            printf("%s%-30s", indent, column.name);
            char typeStr[100];
            switch (column.attrType) {
                case INT:
                    sprintf(typeStr, "int");
                    break;
                case FLOAT:
                    if (column.integerLength || column.decimalLength) {
                        sprintf(typeStr, "decimal(%d,%d)", column.integerLength, column.decimalLength);
                    } else {
                        sprintf(typeStr, "decimal");
                    }
                    break;
                case STRING:
                    sprintf(typeStr, "char(%d)", column.attrLength - 1);
                    break;
                case DATE:
                    sprintf(typeStr, "date");
                    break;
                case VARCHAR:
                    sprintf(typeStr, "varchar(%d)", column.stringMaxLength - 1);
                    break;
                case ATTRARRAY:
                    break;
            }
            printf("%-30s", typeStr);
            printf("%-30s", column.allowNull ? "" : "not null");
            if (column.hasDefaultValue) {
                printf("%s\n", table.formatAttrValueToString(i, column.defaultValue).c_str());
            } else printf("\n");
        }
    }

    printf("%s%s\n", indent, splitLine.data());
    //索引信息展示
    printf("Index\n");
    printf("%s%s\n", indent, splitLine.data());
    printf("%s%-30s%-30s\n", indent, "name", "columns");
    printf("%s%s\n", indent, splitLine.data());
    for (auto &index : dbMeta.tableMetas[tableId].indexes)
        if (index.keyNum) {
            printf("%s%-30s", indent, index.name);
            for (int j = 0; j < index.keyNum; j++) {
                printf("%s%s", GetColumnNameFromId(tableId, index.columnId[j]), j == index.keyNum - 1 ? "" : ",");
            }
            printf("\n");
        }
    printf("%s%s\n", indent, splitLine.data());

    //主键信息展示
    printf("Primary key\n");
    printf("%s", indent);
    if (dbMeta.tableMetas[tableId].primaryKey.keyNum == 0) {
        printf("no primary key\n");
    } else {
        for (int j = 0; j < dbMeta.tableMetas[tableId].primaryKey.keyNum; j++) {
            printf("%s%s", GetColumnNameFromId(tableId, dbMeta.tableMetas[tableId].primaryKey.columnId[j]),
                   j == dbMeta.tableMetas[tableId].primaryKey.keyNum - 1 ? "\n" : ",");
        }
        printf("%s%s\n", indent, splitLine.data());
        printf("%s%-30s%-30s%-30s\n", indent, "table", "name", "foreign key");
        printf("%s%s\n", indent, splitLine.data());
        for (auto &reference : dbMeta.tableMetas[tableId].primaryKey.references)
            if (reference.keyNum) {
                printf("%s", indent);
                printf("%-30s", GetTableNameFromTableId(reference.foreignTable));
                printf("%-30s", reference.name);
                for (int j = 0; j < reference.keyNum; j++) {
                    printf("%s%s", GetColumnNameFromId(reference.foreignTable, reference.foreign[j]),
                           j == reference.keyNum - 1 ? "\n" : ",");
                }
            }
    }

    //外键信息展示
    printf("Foreign key\n");
    printf("%s%s\n", indent, splitLine.data());
    printf("%s%-30s%-30s%-30s%-30s\n", indent, "name", "foreign key", "table", "primary key");
    printf("%s%s\n", indent, splitLine.data());
    for (auto &foreignKey : dbMeta.tableMetas[tableId].foreignKeys)
        if (foreignKey.keyNum) {
            printf("%s", indent);
            printf("%-30s", foreignKey.name);
            for (int j = 0; j < foreignKey.keyNum; j++) {
                printf("%s%s", GetColumnNameFromId(tableId, foreignKey.foreign[j]),
                       j == foreignKey.keyNum - 1 ? "" : ",");
            }
            printf("%-30s", GetTableNameFromTableId(foreignKey.primaryTable));
            for (int j = 0; j < foreignKey.keyNum; j++) {
                printf("%s%s", GetColumnNameFromId(foreignKey.primaryTable, foreignKey.primary[j]),
                       j == foreignKey.keyNum - 1 ? "\n" : ",");
            }
        }
    printf("%s%s\n", indent, splitLine.data());
    //显示表数据的前五行
    printf("Front(<=5) rows\n");
    table.showRecords(5);
    return OK_RC;

}

RC SM_Manager::CreateTable(const char *tbName, std::vector<ColumnDesc> *columnList) {
    DbMeta backup = dbMeta;
    RC rc;
    if (!isUsingDb)return SM_NOT_USING_DATABASE;

    for (int i = 0; i < MAX_TABLE_NUM; i++) {
        if (strlen(dbMeta.tableNames[i]) == 0) {
            strcpy(dbMeta.tableNames[i], tbName);
            TableMeta tableMeta{};
            tableMeta.tableId = i;
            tableMeta.columnNum = columnList->size();
            strcpy(tableMeta.createName, tbName);
            strcpy(tableMeta.name, tbName);
            for (int j = 0; j < columnList->size(); j++) {
                tableMeta.columns[j] = (*columnList)[j];
            }
            SM_Table table(tableMeta);
            //尝试去生成defaultValue的具体值，顺便检查
            for (int j = 0; j < columnList->size(); j++) {
                if (tableMeta.columns[j].hasDefaultValue) {
                    if ((rc = table.completeAttrValueByColumnId(j, tableMeta.columns[j].defaultValue)) != OK_RC) {
                        goto ERROR_EXIT;
                    }
                }
            }
            dbMeta.tableMetas[i] = tableMeta;
            //把主键和外键加进去
            for (auto &column : *columnList) {
                if (column.isPrimaryKey) {
                    auto columns = new std::vector<const char *>{column.name};
                    if ((rc = AddPrimaryKey(tbName, columns)) != OK_RC) {
                        delete columns;
                        goto ERROR_EXIT;
                    }
                    delete columns;
                }
                if (column.hasForeignKey) {
                    auto foreignColumns = new std::vector<const char *>{column.name};
                    auto primaryColumns = new std::vector<const char *>{column.primaryKeyColumn};
                    if ((rc = AddForeignKey(column.name, tbName, foreignColumns, column.primaryKeyTable,
                                            primaryColumns)) != OK_RC) {
                        delete foreignColumns;
                        delete primaryColumns;
                        goto ERROR_EXIT;
                    }
                    delete foreignColumns;
                    delete primaryColumns;
                }
            }
            WriteDbMeta();
            return OK_RC;
        } else if (strcmp(dbMeta.tableNames[i], tbName) == 0) {
            return SM_TABLE_ALREADY_IN;
        }
    }
    return SM_TABLE_IS_FULL;
    ERROR_EXIT:
    dbMeta = backup;
    WriteDbMeta();
    return rc;
}

RC SM_Manager::ReadDbMeta() {
    //不存在的话要新建
    if (access("DB.Meta", F_OK) < 0) {
        memset(&dbMeta, 0, sizeof(dbMeta));
        WriteDbMeta();
    }
    FILE *file = fopen("DB.Meta", "r");
    if (fread(&dbMeta, sizeof(dbMeta), 1, file) != 1) {
        return SM_UNIX;
    }
    fclose(file);
    return OK_RC;
}

RC SM_Manager::WriteDbMeta() {
    FILE *file = fopen("DB.Meta", "w");
    if (fwrite(&dbMeta, sizeof(dbMeta), 1, file) != 1) {
        return SM_UNIX;
    }
    fclose(file);
    return OK_RC;
}

const char *SM_Manager::GetTableNameFromTableId(TableId tableId) {
    return dbMeta.tableNames[tableId];
}

const char *SM_Manager::GetColumnNameFromId(TableId tableId, ColumnId columnId) {
    return dbMeta.tableMetas[tableId].columns[columnId].name;
}

RC SM_Manager::AddPrimaryKey(const char *tbName, std::vector<const char *> *columns) {
    TableId tableId = GetTableIdFromName(tbName);
    if (tableId < 0)return SM_TABLE_NOT_EXIST;
    //必须先drop掉原来的才可以建新的
    if (dbMeta.tableMetas[tableId].primaryKey.keyNum != 0)return SM_ALREADY_HAVE_PRIMARY_KEY;
    PrimaryKeyDesc primaryKey{};
    primaryKey.keyNum = columns->size();
    for (int i = 0; i < columns->size(); i++) {
        primaryKey.columnId[i] = GetColumnIdFromName(tableId, (*columns)[i]);
        if (primaryKey.columnId[i] < 0) {
            return SM_COLUMN_NOT_EXIST;
        }
    }
    //找到索引空位，然后尝试插入索引，同时判断是否存在有空
    for (int i = 0; i < MAX_INDEX_NUM; i++)
        if (dbMeta.tableMetas[tableId].indexes[i].keyNum == 0) {
            IndexDesc indexDesc{};
            strcpy(indexDesc.name, "primary key");
            indexDesc.keyNum = primaryKey.keyNum;
            for (int j = 0; j < primaryKey.keyNum; j++) {
                indexDesc.columnId[j] = primaryKey.columnId[j];
            }
            primaryKey.indexIndex = i;
            SM_Table table(tableId);
            TRY(table.createIndex(i, indexDesc, false));
            dbMeta.tableMetas[tableId].indexes[i] = indexDesc;
            dbMeta.tableMetas[tableId].primaryKey = primaryKey;
            WriteDbMeta();
            return OK_RC;
        }
    return SM_INDEX_IS_FULL;
}

RC
SM_Manager::AddForeignKey(const char *name, const char *foreignTable, std::vector<const char *> *foreignColumns,
                          const char *primaryTable,
                          std::vector<const char *> *primaryColumns) {
    TableId foreignTableId = GetTableIdFromName(foreignTable);
    if (foreignTableId < 0)return SM_TABLE_NOT_EXIST;
    TableId primaryTableId = GetTableIdFromName(primaryTable);
    if (primaryTableId < 0)return SM_TABLE_NOT_EXIST;
    if (foreignColumns->size() != primaryColumns->size())return SM_FOREIGN_KEYS_AND_PRIMARY_KEYS_NOT_MATCH;
    ForeignKeyDesc foreignKey{};
    strcpy(foreignKey.name, name);
    foreignKey.foreignTable = foreignTableId;
    foreignKey.primaryTable = primaryTableId;
    foreignKey.keyNum = foreignColumns->size();
    for (int k = 0; k < foreignKey.keyNum; k++) {
        foreignKey.foreign[k] = GetColumnIdFromName(foreignTableId, (*foreignColumns)[k]);
        foreignKey.primary[k] = GetColumnIdFromName(primaryTableId, (*primaryColumns)[k]);
        if (foreignKey.foreign[k] < 0 || foreignKey.primary[k] < 0) {
            return SM_COLUMN_NOT_EXIST;
        }
    }
    //检查引用的是不是唯一主键
    PrimaryKeyDesc &primaryKey = dbMeta.tableMetas[primaryTableId].primaryKey;
    if (foreignKey.keyNum != primaryKey.keyNum)return SM_PRIMARY_KEY_NOT_EXIST;
    for (int k = 0; k < foreignKey.keyNum; k++) {
        if (foreignKey.primary[k] != primaryKey.columnId[k]) {
            return SM_PRIMARY_KEY_NOT_EXIST;
        }
    }
    //先把索引拿出来
    IndexDesc index{};
    for (int i = 0; i < MAX_INDEX_NUM; i++)
        if (dbMeta.tableMetas[foreignTableId].indexes[i].keyNum == 0) {
            //注意这里，索引的名字和外键的名字是一样的
            strcpy(index.name, name);
            index.keyNum = foreignKey.keyNum;
            for (int j = 0; j < foreignKey.keyNum; j++) {
                index.columnId[j] = foreignKey.foreign[j];
            }
            foreignKey.indexIndex = i;
        }
    //检查所有记录里有没有链接到空的主键上去的记录
    SM_Table table(foreignTableId);
    if (!table.validForeignKey(index, primaryTableId))return QL_NO_INDICATE_PRIMARY_KEY_EXIST;
    //有同名的直接返回
    for (auto &k : dbMeta.tableMetas[foreignTableId].foreignKeys)
        if (strcmp(k.name, name) == 0)
            return SM_FOREIGN_KEY_ALREADY_EXIST;
    //先找到空的外键位置
    for (auto &k : dbMeta.tableMetas[foreignTableId].foreignKeys)
        if (k.keyNum == 0) {
            //再找到空的链接位置
            for (auto &reference : dbMeta.tableMetas[primaryTableId].primaryKey.references)
                if (reference.keyNum == 0) {
                    //各类信息归位
                    TRY(table.createIndex(foreignKey.indexIndex, index, false));
                    dbMeta.tableMetas[foreignTableId].indexes[foreignKey.indexIndex] = index;
                    k = foreignKey;
                    reference = foreignKey;
                    WriteDbMeta();
                    return OK_RC;
                }
            return SM_REFERENCE_IS_FULL;
        }
    return SM_FOREIGN_KEYS_IS_FULL;
}

TableId SM_Manager::GetTableIdFromName(const char *tableName) {
    for (int i = 0; i < MAX_TABLE_NUM; i++)
        if (strcmp(dbMeta.tableNames[i], tableName) == 0) {
            return i;
        }
    return -1;
}

ColumnId SM_Manager::GetColumnIdFromName(const char *tableName, const char *columnName) {
    TableId tableId = GetTableIdFromName(tableName);
    return GetColumnIdFromName(tableId, columnName);
}

ColumnId SM_Manager::GetColumnIdFromName(TableId tableId, const char *columnName) {
    if (tableId < 0)return -1;
    for (int i = 0; i < MAX_COLUMN_NUM; i++)
        if (strcmp(dbMeta.tableMetas[tableId].columns[i].name, columnName) == 0) {
            return i;
        }
    return -1;
}

RC SM_Manager::DropPrimaryKey(const char *table) {
    TableId primaryTableId = GetTableIdFromName(table);
    if (primaryTableId < 0)return SM_PRIMARY_KEY_NOT_EXIST;
    //主键先根据引用把外键一条条删除，最后再删除主键
    for (auto &reference:dbMeta.tableMetas[primaryTableId].primaryKey.references) {
        if (reference.keyNum) {
            return SM_PRIMARY_KEY_HAS_FOREIGN_KEY_DEPENDENCY;
        }
    }
    if (dbMeta.tableMetas[primaryTableId].primaryKey.keyNum) {
        int primaryIndex = dbMeta.tableMetas[primaryTableId].primaryKey.indexIndex;
        //删除主键的索引以及索引记录
        TRY(IX_Manager::Instance().DestroyIndex(dbMeta.tableMetas[primaryTableId].createName, primaryIndex))
        memset(&dbMeta.tableMetas[primaryTableId].indexes[primaryIndex], 0, sizeof(IndexDesc));
        memset(&dbMeta.tableMetas[primaryTableId].primaryKey, 0, sizeof(PrimaryKeyDesc));
    } else {
        return SM_PRIMARY_KEY_NOT_EXIST;
    }
    return OK_RC;
}

RC SM_Manager::DropForeignKey(const char *foreignTable, const char *name) {
    TableId foreignTableId = GetTableIdFromName(foreignTable);
    if (foreignTableId < 0)return SM_TABLE_NOT_EXIST;
    //不仅要删除外键还要删除主键中记录的引用
    for (auto &foreignKey : dbMeta.tableMetas[foreignTableId].foreignKeys)
        if (strcmp(foreignKey.name, name) == 0) {
            for (auto &reference: dbMeta.tableMetas[foreignKey.primaryTable].primaryKey.references)
                if (strcmp(reference.name, name) == 0) {
                    memset(&reference, 0, sizeof(reference));
                    break;
                }
            //删除外键的索引以及索引记录
            int foreignIndex = foreignKey.indexIndex;
            TRY(IX_Manager::Instance().DestroyIndex(dbMeta.tableMetas[foreignTableId].createName, foreignIndex))
            memset(&dbMeta.tableMetas[foreignTableId].indexes[foreignIndex], 0, sizeof(IndexDesc));
            //清空外键记录以及写入
            memset(&foreignKey, 0, sizeof(foreignKey));
            WriteDbMeta();
            return OK_RC;
        }
    return SM_FOREIGN_KEY_NOT_EXIST;
}

TableMeta &SM_Manager::GetTableMeta(TableId tableId) {
    return dbMeta.tableMetas[tableId];
}

RC SM_Manager::ShowTable(const char *tbName) {
    TableId tableId = GetTableIdFromName(tbName);
    if (tableId < 0)return SM_TABLE_NOT_EXIST;
    SM_Table table(dbMeta.tableMetas[tableId]);
    table.showRecords(-1);
    return OK_RC;
}

RC SM_Manager::AddIndex(const char *tbName, const char *indexName, std::vector<const char *> *columns) {
    TableId tableId = GetTableIdFromName(tbName);
    if (tableId < 0)return SM_TABLE_NOT_EXIST;
    //先拷贝到一个index上
    IndexDesc indexDesc;
    indexDesc.keyNum = columns->size();
    for (int i = 0; i < columns->size(); i++) {
        indexDesc.columnId[i] = GetColumnIdFromName(tableId, (*columns)[i]);
        if (indexDesc.columnId[i] < 0) {
            return SM_COLUMN_NOT_EXIST;
        }
    }
    strcpy(indexDesc.name, indexName);

    //先看一遍是否有重名的index，有的话不允许添加
    for (auto &index:dbMeta.tableMetas[tableId].indexes)
        if (strcmp(index.name, indexName) == 0)
            return SM_INDEX_ALREADY_IN;
    //没有的话再找空位往里加
    for (int i = 0; i < MAX_INDEX_NUM; i++)
        if (dbMeta.tableMetas[tableId].indexes[i].keyNum == 0) {
            SM_Table table(tableId);
            TRY(table.createIndex(i, indexDesc));
            dbMeta.tableMetas[tableId].indexes[i] = indexDesc;
            WriteDbMeta();
            return OK_RC;
        }
    return SM_INDEX_IS_FULL;
}

RC SM_Manager::DropIndex(const char *tbName, const char *indexName) {
    TableId tableId = GetTableIdFromName(tbName);
    if (tableId < 0)return SM_TABLE_NOT_EXIST;
    for (int i = 0; i < MAX_INDEX_NUM; i++)
        if (strcmp(dbMeta.tableMetas[tableId].indexes[i].name, indexName) == 0) {
            TRY(ixManager.DestroyIndex(dbMeta.tableMetas[tableId].createName, i))
            memset(&dbMeta.tableMetas[tableId].indexes[i], 0, sizeof(IndexDesc));
            return OK_RC;
        }
    return SM_INDEX_NOT_EXIST;
}

AttrType SM_Manager::GetType(TableId tableId, ColumnId columnId) {
    if (tableId < 0 || columnId < 0)return BOOL;
    return dbMeta.tableMetas[tableId].columns[columnId].attrType;
}

TableMeta &SM_Manager::GetTableMeta(const char *tbName) {
    TableId tableId = GetTableIdFromName(tbName);
    return GetTableMeta(tableId);
}


