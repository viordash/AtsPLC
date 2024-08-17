#include "CppUTest/TestHarness_c.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static uint8_t rtc_memory[256] = {};

TEST_GROUP_C_SETUP(RestartCounterTestsGroup) {
    memset(rtc_memory, 0, sizeof(rtc_memory));
}
TEST_GROUP_C_TEARDOWN(RestartCounterTestsGroup) {
}

#define RTC_USER_BASE rtc_memory
#include "main/restart_counter.c"

TEST_C(RestartCounterTestsGroup, hot_restart_counter) {
    load_hotreload();

    hot_restart_counter();

    CHECK_EQUAL_C_UINT(1, hotreload->restart_count);

    hot_restart_counter();
    hot_restart_counter();

    CHECK_EQUAL_C_UINT(3, hotreload->restart_count);
}