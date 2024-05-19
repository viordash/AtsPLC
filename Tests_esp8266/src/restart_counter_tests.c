#include "CppUTest/TestHarness_c.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static uint8_t testable[256] = {};

TEST_GROUP_C_SETUP(RestartCounterTestsGroup) {
    memset(testable, 0, sizeof(testable));
}
TEST_GROUP_C_TEARDOWN(RestartCounterTestsGroup) {
}

#define RTC_USER_BASE testable
#include "restart_counter.c"

TEST_C(RestartCounterTestsGroup, hot_restart) {
    hotreload load_data = {};

    CHECK_EQUAL_C_BOOL(false, try_load_hotreload(&load_data));

    hot_restart();

    CHECK_EQUAL_C_BOOL(true, try_load_hotreload(&load_data));
    CHECK_EQUAL_C_UINT(1, load_data.restart_count);

    hot_restart();
    hot_restart();

    CHECK_EQUAL_C_BOOL(true, try_load_hotreload(&load_data));
    CHECK_EQUAL_C_UINT(3, load_data.restart_count);
}