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
    settings.wifi_scanner.max_rssi = 1;
    settings.wifi_scanner.min_rssi = 1;
    settings.wifi_access_point.generation_time_ms = 1;
    settings.wifi_access_point.ssid_hidden = true;
    load_settings();

    CHECK_EQUAL(0, settings.smartconfig.counter);
    STRCMP_EQUAL("", settings.wifi_station.ssid);
    STRCMP_EQUAL("", settings.wifi_station.password);
    CHECK_EQUAL(-26, settings.wifi_scanner.max_rssi);
    CHECK_EQUAL(-120, settings.wifi_scanner.min_rssi);
    CHECK_EQUAL(20000, settings.wifi_access_point.generation_time_ms);
    CHECK_FALSE(settings.wifi_access_point.ssid_hidden);

    STRCMP_EQUAL("ru.pool.ntp.org", settings.datetime.sntp_server_primary);
    STRCMP_EQUAL("pool.ntp.org", settings.datetime.sntp_server_secondary);
    STRCMP_EQUAL("GMT-3", settings.datetime.timezone);
}

TEST(SettingsTestsGroup, store_load_settings) {
    settings.smartconfig.counter = 42;
    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");
    settings.wifi_scanner.max_rssi = 100;
    settings.wifi_scanner.min_rssi = -100;
    settings.wifi_access_point.generation_time_ms = 90123;
    settings.wifi_access_point.ssid_hidden = true;
    store_settings();

    memset(&settings, 0, sizeof(settings));
    load_settings();

    CHECK_EQUAL(42, settings.smartconfig.counter);
    STRCMP_EQUAL("test_ssid", settings.wifi_station.ssid);
    STRCMP_EQUAL("test_pwd", settings.wifi_station.password);
    CHECK_EQUAL(100, settings.wifi_scanner.max_rssi);
    CHECK_EQUAL(-100, settings.wifi_scanner.min_rssi);
    CHECK_EQUAL(90123, settings.wifi_access_point.generation_time_ms);
    CHECK_TRUE(settings.wifi_access_point.ssid_hidden);
}

TEST(SettingsTestsGroup, delete_settings) {
    settings.smartconfig.counter = 42;
    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");
    settings.wifi_scanner.max_rssi = 100;
    settings.wifi_scanner.min_rssi = -100;
    settings.wifi_access_point.generation_time_ms = 90123;
    settings.wifi_access_point.ssid_hidden = true;
    store_settings();

    CHECK_TRUE(storage_0_exists(settings_storage_name));
    CHECK_TRUE(storage_1_exists(settings_storage_name));

    delete_settings();

    CHECK_FALSE(storage_0_exists(settings_storage_name));
    CHECK_FALSE(storage_1_exists(settings_storage_name));

    memset(&settings, 0, sizeof(settings));
    load_settings();

    CHECK_EQUAL(0, settings.smartconfig.counter);
    STRCMP_EQUAL("", settings.wifi_station.ssid);
    STRCMP_EQUAL("", settings.wifi_station.password);
    CHECK_EQUAL(-26, settings.wifi_scanner.max_rssi);
    CHECK_EQUAL(-120, settings.wifi_scanner.min_rssi);
    CHECK_EQUAL(20000, settings.wifi_access_point.generation_time_ms);
    CHECK_FALSE(settings.wifi_access_point.ssid_hidden);
}

TEST(SettingsTestsGroup, validate_settings) {
    load_settings();

    CHECK_TRUE(validate_settings(&settings));

    settings.wifi_station.ssid[0] = 0;
    CHECK_TRUE(validate_settings(&settings));
    settings.wifi_station.ssid[0] = '!' - 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.ssid[0] = '!';
    CHECK_TRUE(validate_settings(&settings));
    settings.wifi_station.ssid[0] = '~' + 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.ssid[0] = '~';
    CHECK_TRUE(validate_settings(&settings));

    settings.wifi_station.password[0] = 0;
    CHECK_TRUE(validate_settings(&settings));
    settings.wifi_station.password[0] = '!' - 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.password[0] = '!';
    CHECK_TRUE(validate_settings(&settings));
    settings.wifi_station.password[0] = '~' + 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.password[0] = '~';
    CHECK_TRUE(validate_settings(&settings));

    settings.wifi_station.connect_max_retry_count = -1 - 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.connect_max_retry_count = 7777 + 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.connect_max_retry_count = -1;
    CHECK_TRUE(validate_settings(&settings));
    settings.wifi_station.connect_max_retry_count = 7777;
    CHECK_TRUE(validate_settings(&settings));

    settings.wifi_station.reconnect_delay_ms = 100 - 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.reconnect_delay_ms = 10 * 60 * 1000 + 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.reconnect_delay_ms = 100;
    CHECK_TRUE(validate_settings(&settings));
    settings.wifi_station.reconnect_delay_ms = 10 * 60 * 1000;
    CHECK_TRUE(validate_settings(&settings));

    settings.wifi_station.scan_station_rssi_period_ms = 100 - 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.scan_station_rssi_period_ms = 10 * 60 * 1000 + 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.scan_station_rssi_period_ms = 100;
    CHECK_TRUE(validate_settings(&settings));
    settings.wifi_station.scan_station_rssi_period_ms = 10 * 60 * 1000;
    CHECK_TRUE(validate_settings(&settings));

    settings.wifi_station.max_rssi = -120 - 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.max_rssi = 100 + 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.max_rssi = -120;
    CHECK_TRUE(validate_settings(&settings));
    settings.wifi_station.max_rssi = 100;
    CHECK_TRUE(validate_settings(&settings));

    settings.wifi_station.min_rssi = -120 - 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.min_rssi = 100 + 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.min_rssi = -120;
    CHECK_TRUE(validate_settings(&settings));
    settings.wifi_station.min_rssi = 100;
    CHECK_TRUE(validate_settings(&settings));

    settings.wifi_station.max_rssi = -10;
    settings.wifi_station.min_rssi = -9;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_station.min_rssi = -10;
    CHECK_TRUE(validate_settings(&settings));

    settings.wifi_scanner.per_channel_scan_time_ms = 100 - 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_scanner.per_channel_scan_time_ms = 20 * 1000 + 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_scanner.per_channel_scan_time_ms = 100;
    CHECK_TRUE(validate_settings(&settings));
    settings.wifi_scanner.per_channel_scan_time_ms = 20 * 1000;
    CHECK_TRUE(validate_settings(&settings));

    settings.wifi_scanner.max_rssi = -120 - 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_scanner.max_rssi = 100 + 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_scanner.max_rssi = -120;
    CHECK_TRUE(validate_settings(&settings));
    settings.wifi_scanner.max_rssi = 100;
    CHECK_TRUE(validate_settings(&settings));

    settings.wifi_scanner.min_rssi = -120 - 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_scanner.min_rssi = 100 + 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_scanner.min_rssi = -120;
    CHECK_TRUE(validate_settings(&settings));
    settings.wifi_scanner.min_rssi = 100;
    CHECK_TRUE(validate_settings(&settings));

    settings.wifi_scanner.max_rssi = -10;
    settings.wifi_scanner.min_rssi = -9;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_scanner.min_rssi = -10;
    CHECK_TRUE(validate_settings(&settings));

    settings.wifi_access_point.generation_time_ms = 100 - 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_access_point.generation_time_ms = 10 * 60 * 1000 + 1;
    CHECK_FALSE(validate_settings(&settings));
    settings.wifi_access_point.generation_time_ms = 100;
    CHECK_TRUE(validate_settings(&settings));
    settings.wifi_access_point.generation_time_ms = 10 * 60 * 1000;
    CHECK_TRUE(validate_settings(&settings));

    *((char *)&settings.wifi_access_point.ssid_hidden) = 2;
    CHECK_FALSE(validate_settings(&settings));
    *((char *)&settings.wifi_access_point.ssid_hidden) = 0;
    CHECK_TRUE(validate_settings(&settings));
    *((char *)&settings.wifi_access_point.ssid_hidden) = 1;
    CHECK_TRUE(validate_settings(&settings));

    settings.adc.scan_period_ms = 19;
    CHECK_FALSE(validate_settings(&settings));
    settings.adc.scan_period_ms = 60001;
    CHECK_FALSE(validate_settings(&settings));
    settings.adc.scan_period_ms = 20;
    CHECK_TRUE(validate_settings(&settings));
    settings.adc.scan_period_ms = 60000;
    CHECK_TRUE(validate_settings(&settings));
}
