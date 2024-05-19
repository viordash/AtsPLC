#include "CppUTest/TestHarness_c.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static uint8_t testable[256] = {};

TEST_GROUP_C_SETUP(HotReloadTestsGroup) {
    memset(testable, 0, sizeof(testable));
}
TEST_GROUP_C_TEARDOWN(HotReloadTestsGroup) {
}

#define RTC_USER_BASE testable
#include "hotreload_service.c"

TEST_C(HotReloadTestsGroup, store_load) {
    hotreload store_data = {};
    hotreload load_data = {};

    store_data.gpio = 42;
    store_data.restart_count = 19;

    store_hotreload(&store_data);

    CHECK_EQUAL_C_BOOL(true, try_load_hotreload(&load_data));

    CHECK_EQUAL_C_UINT(store_data.gpio, load_data.gpio);
    CHECK_EQUAL_C_UINT(store_data.restart_count, load_data.restart_count);
}

TEST_C(HotReloadTestsGroup, load_if_memory_cleared) {
    hotreload load_data = {};

    CHECK_EQUAL_C_BOOL(false, try_load_hotreload(&load_data));
}