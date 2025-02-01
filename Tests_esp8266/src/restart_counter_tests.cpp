#include "CppUTest/TestHarness.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "main/restart_counter.cpp"

TEST_GROUP(RestartCounterTestsGroup){ //
                                      TEST_SETUP(){}

                                      TEST_TEARDOWN(){}
};

TEST(RestartCounterTestsGroup, hot_restart_counter) {
    load_hotreload();

    hot_restart_counter();

    CHECK_EQUAL(1, hotreload->restart_count);

    hot_restart_counter();
    hot_restart_counter();

    CHECK_EQUAL(3, hotreload->restart_count);
}