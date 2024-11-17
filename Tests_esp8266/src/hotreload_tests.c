#include "CppUTest/TestHarness_c.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static uint8_t rtc_memory[256] = {};

TEST_GROUP_C_SETUP(HotReloadTestsGroup) {
    memset(rtc_memory, 0, sizeof(rtc_memory));
}
TEST_GROUP_C_TEARDOWN(HotReloadTestsGroup) {
}

#define RTC_USER_BASE rtc_memory
#include "main/hotreload_service.c"

TEST_C(HotReloadTestsGroup, load_store) {
    rtc_hotreload_data *testable = (rtc_hotreload_data *)rtc_memory;

    load_hotreload();

    hotreload->is_hotstart = true;
    hotreload->restart_count = 19;
    hotreload->view_top_index = 75;
    hotreload->selected_network = 7;

    store_hotreload();

    CHECK_EQUAL_C_UINT(0xDE4572BB, testable->magic);
    CHECK_EQUAL_C_BOOL(true, testable->data.is_hotstart);
    CHECK_EQUAL_C_UINT(19, testable->data.restart_count);
    CHECK_EQUAL_C_UINT(75, testable->data.view_top_index);
    CHECK_EQUAL_C_UINT(7, testable->data.selected_network);
}

TEST_C(HotReloadTestsGroup, load_if_memory_cleared) {

    rtc_hotreload_data *testable = (rtc_hotreload_data *)rtc_memory;

    load_hotreload();

    CHECK_EQUAL_C_UINT(0, testable->magic);
    CHECK_EQUAL_C_BOOL(false, testable->data.is_hotstart);
    CHECK_EQUAL_C_UINT(0, testable->data.restart_count);
    CHECK_EQUAL_C_UINT(0, testable->data.view_top_index);
    CHECK_EQUAL_C_UINT(-1, testable->data.selected_network);
}