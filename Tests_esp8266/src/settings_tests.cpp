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
    load_settings();

    CHECK_EQUAL(0, settings.smartconfig.counter);
    STRCMP_EQUAL("", settings.wifi_station.ssid);
    STRCMP_EQUAL("", settings.wifi_station.password);
}

TEST(SettingsTestsGroup, store_load_settings) {
    settings.smartconfig.counter = 42;
    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");
    store_settings();

    memset(&settings, 0, sizeof(settings));
    load_settings();

    CHECK_EQUAL(42, settings.smartconfig.counter);
    STRCMP_EQUAL("test_ssid", settings.wifi_station.ssid);
    STRCMP_EQUAL("test_pwd", settings.wifi_station.password);
}
