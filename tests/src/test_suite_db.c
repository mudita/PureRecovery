// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "test_suite_db.h"
#include <seatest/seatest.h>
#include <common/database.h>
#include <hal/delay.h>
#include <stdint.h>

static void measure_time(const uint32_t *t1) {
    printf("%lu ms\n", get_jiffiess() - *t1);
}

#define MEASURE_TIME(var) const uint32_t var __attribute__((__cleanup__(measure_time)))

static void close_db(database_t **db) {
    if (*db != NULL) {
        database_close(*db);
    }
}

/// Create fresh database with one table "test" containing 1000 entries
static database_t *create_test_db(const char *path) {
    remove(path);
    database_t *db = database_open(path);
    assert_true(db != NULL);
    assert_true(database_execute(db, "create table if not exists test(name text, surname text, height INT);"
                                     "create trigger test_ins_trigger after insert on test"
                                     "  when new.height < 1000 begin"
                                     "    insert into test(name,surname,height) values('Name','Surname',new.height +1);"
                                     "  end;"
                                     "pragma recursive_triggers = 1;"
                                     "insert into test(name,surname,height) values('Name','Surname',1);") == Db_Ok);
    return db;
}

#define DB_HANDLE(var) database_t * var __attribute__((__cleanup__(close_db)))

static void create_table(void) {
    printf("Executing %s took ", __func__);
    MEASURE_TIME(t1) = get_jiffiess();
    DB_HANDLE(db) = create_test_db("/user/test.db");
    assert_true(db != NULL);
}

static void alter_table_rename(void) {
    printf("Executing %s took ", __func__);
    DB_HANDLE(db) = create_test_db("/user/test.db");
    assert_true(db != NULL);

    MEASURE_TIME(t1) = get_jiffiess();
    assert_true(database_execute(db, "alter table test rename to test2;") == Db_Ok);
}

static void create_new_table(void) {
    printf("Executing %s took ", __func__);
    DB_HANDLE(db) = create_test_db("/user/test.db");
    assert_true(db != NULL);

    MEASURE_TIME(t1) = get_jiffiess();
    assert_true(database_execute(db, "create table new_table(name,surname,height,age);") == Db_Ok);
}

static void create_and_copy_table(void) {
    printf("Executing %s took ", __func__);
    DB_HANDLE(db) = create_test_db("/user/test.db");
    assert_true(db != NULL);

    MEASURE_TIME(t1) = get_jiffiess();
    assert_true(database_execute(db, "create table new_table(name,surname,height,age);"
                                     "insert into new_table(name,surname,height) SELECT name,surname,height from test;") ==
                Db_Ok);
}

static void add_column(void) {
    printf("Executing %s took ", __func__);
    DB_HANDLE(db) = create_test_db("/user/test.db");
    assert_true(db != NULL);

    MEASURE_TIME(t1) = get_jiffiess();
    assert_true(database_execute(db, "alter table test ADD age int DEFAULT 30;") == Db_Ok);
}

static void update_values_in_column(void) {
    printf("Executing %s took ", __func__);
    DB_HANDLE(db) = create_test_db("/user/test.db");
    assert_true(db != NULL);

    MEASURE_TIME(t1) = get_jiffiess();
    assert_true(database_execute(db, "update test set surname='Surname2';") == Db_Ok);
}


void test_suite_db(void) {
    database_initialize();
    test_fixture_start();
    run_test(create_table);
    run_test(alter_table_rename);
    run_test(create_new_table);
    run_test(create_and_copy_table);
    run_test(add_column);
    run_test(update_values_in_column);
    test_fixture_end();
    database_deinitialize();
}
