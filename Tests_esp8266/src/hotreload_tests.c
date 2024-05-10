#include "CppUTest/TestHarness_c.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

static uint8_t testable[256] = {};
#define RTC_USER_BASE testable

#include "hotreload_service.c"

TEST_C(HotReloadTestsGroup, store_load) {
    hotreload store_data = {};
    hotreload load_data = {};

    store_data.gpio = 42;

    store_hotreload(&store_data);

    CHECK_EQUAL_C_BOOL(true, try_load_hotreload(&load_data));

    CHECK_EQUAL_C_MEMCMP(&store_data, &load_data, sizeof(store_data));
}