#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/MigrateAnyData/MigrateAnyData.cpp"
#include "main/settings.cpp"

#include "tests_utils.h"

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
    strcpy(settings.wifi.ssid, "test_ssid");
    strcpy(settings.wifi.password, "test_pwd");
    settings.state = 19;
    load_settings();

    CHECK_EQUAL(0, settings.smartconfig.counter);
    STRCMP_EQUAL("", settings.wifi.ssid);
    STRCMP_EQUAL("", settings.wifi.password);
    CHECK_EQUAL(0xFF, settings.state);
}

TEST(SettingsTestsGroup, store_load_settings) {
    settings.smartconfig.counter = 42;
    strcpy(settings.wifi.ssid, "test_ssid");
    strcpy(settings.wifi.password, "test_pwd");
    settings.state = 19;
    store_settings();

    settings.smartconfig.counter = 0;
    strcpy(settings.wifi.ssid, "");
    strcpy(settings.wifi.password, "");
    settings.state = 0;
    load_settings();

    CHECK_EQUAL(42, settings.smartconfig.counter);
    STRCMP_EQUAL("test_ssid", settings.wifi.ssid);
    STRCMP_EQUAL("test_pwd", settings.wifi.password);
    CHECK_EQUAL(19, settings.state);
}
