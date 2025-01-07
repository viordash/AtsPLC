#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "tests_utils.h"

#include "main/settings.h"

TEST_GROUP(SettingsTestsGroup){ //
                                TEST_SETUP(){ create_storage_0();
create_storage_1();
mock().disable();
}

TEST_TEARDOWN() {
    remove_storage_0();
    remove_storage_1();
    mock().enable();
}
}
;

TEST(SettingsTestsGroup, load_if_clear_storage_return_default_settings) {
    settings.smartconfig.counter = 42;
    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");
    settings.wifi_scanner.delay_re_adding_request_ms = 1;
    settings.wifi_scanner.max_rssi = 1;
    settings.wifi_scanner.min_rssi = 1;
    settings.wifi_access_point.delay_re_adding_request_ms = 1;
    settings.wifi_access_point.generation_time_ms = 1;
    settings.wifi_access_point.ssid_hidden = true;
    load_settings();

    CHECK_EQUAL(0, settings.smartconfig.counter);
    STRCMP_EQUAL("", settings.wifi_station.ssid);
    STRCMP_EQUAL("", settings.wifi_station.password);
    CHECK_EQUAL(3000, settings.wifi_scanner.delay_re_adding_request_ms);
    CHECK_EQUAL(-26, settings.wifi_scanner.max_rssi);
    CHECK_EQUAL(-120, settings.wifi_scanner.min_rssi);
    CHECK_EQUAL(3000, settings.wifi_access_point.delay_re_adding_request_ms);
    CHECK_EQUAL(20000, settings.wifi_access_point.generation_time_ms);
    CHECK_FALSE(settings.wifi_access_point.ssid_hidden);
}

TEST(SettingsTestsGroup, store_load_settings) {
    settings.smartconfig.counter = 42;
    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");
    settings.wifi_scanner.delay_re_adding_request_ms = 1234;
    settings.wifi_scanner.max_rssi = 100;
    settings.wifi_scanner.min_rssi = -100;
    settings.wifi_access_point.delay_re_adding_request_ms = 5678;
    settings.wifi_access_point.generation_time_ms = 90123;
    settings.wifi_access_point.ssid_hidden = true;
    store_settings();

    memset(&settings, 0, sizeof(settings));
    load_settings();

    CHECK_EQUAL(42, settings.smartconfig.counter);
    STRCMP_EQUAL("test_ssid", settings.wifi_station.ssid);
    STRCMP_EQUAL("test_pwd", settings.wifi_station.password);
    CHECK_EQUAL(1234, settings.wifi_scanner.delay_re_adding_request_ms);
    CHECK_EQUAL(100, settings.wifi_scanner.max_rssi);
    CHECK_EQUAL(-100, settings.wifi_scanner.min_rssi);
    CHECK_EQUAL(5678, settings.wifi_access_point.delay_re_adding_request_ms);
    CHECK_EQUAL(90123, settings.wifi_access_point.generation_time_ms);
    CHECK_TRUE(settings.wifi_access_point.ssid_hidden);
}
