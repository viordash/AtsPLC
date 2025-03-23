#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "tests_utils.h"

#include "main/LogicProgram/Settings/SettingsElement.cpp"
#include "main/LogicProgram/Settings/SettingsElement.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicSettingsElementTestsGroup){
    //
    TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
create_storage_0();
create_storage_1();
mock().disable();
Controller::Start(NULL, NULL, NULL);
load_settings();
}

TEST_TEARDOWN() {
    Controller::Stop();
    remove_storage_0();
    remove_storage_1();
    mock().enable();
}
}
;

namespace {
    class TestableSettingsElement : public SettingsElement {
      public:
        TestableSettingsElement() : SettingsElement() {
        }
        int *PublicMorozov_Get_editing_property_id() {
            return &editing_property_id;
        }
        bool PublicMorozov_ValidateDiscriminator(Discriminator *discriminator) {
            return ValidateDiscriminator(discriminator);
        }
        Discriminator *PublicMorozov_Get_discriminator() {
            return &discriminator;
        }
        void PublicMorozov_ReadValue(char *string_buffer, bool friendly_format) {
            ReadValue(string_buffer, friendly_format);
        }
        char *PublicMorozov_Get_value() {
            return value;
        }
    };
} // namespace

TEST(LogicSettingsElementTestsGroup, Render) {
    TestableSettingsElement testable;

    Point start_point = { INCOME_RAIL_WIDTH, INCOME_RAIL_TOP + INCOME_RAIL_NETWORK_TOP };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(113, start_point.x);
}

TEST(LogicSettingsElementTestsGroup, ValidateDiscriminator) {
    TestableSettingsElement testable;
    SettingsElement::Discriminator discriminator =
        SettingsElement::Discriminator::t_wifi_station_settings_ssid;
    CHECK_TRUE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
    discriminator = SettingsElement::Discriminator::t_wifi_station_settings_password;
    CHECK_TRUE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
    discriminator = SettingsElement::Discriminator::t_wifi_station_settings_connect_max_retry_count;
    CHECK_TRUE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
    discriminator = SettingsElement::Discriminator::t_wifi_station_settings_reconnect_delay_ms;
    CHECK_TRUE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
    discriminator =
        SettingsElement::Discriminator::t_wifi_station_settings_scan_station_rssi_period_ms;
    CHECK_TRUE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
    discriminator = SettingsElement::Discriminator::t_wifi_station_settings_max_rssi;
    CHECK_TRUE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
    discriminator = SettingsElement::Discriminator::t_wifi_station_settings_min_rssi;
    CHECK_TRUE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
    discriminator =
        SettingsElement::Discriminator::t_wifi_scanner_settings_per_channel_scan_time_ms;
    CHECK_TRUE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
    discriminator = SettingsElement::Discriminator::t_wifi_scanner_settings_max_rssi;
    CHECK_TRUE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
    discriminator = SettingsElement::Discriminator::t_wifi_scanner_settings_min_rssi;
    CHECK_TRUE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
    discriminator = SettingsElement::Discriminator::t_wifi_access_point_settings_generation_time_ms;
    CHECK_TRUE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
    discriminator = SettingsElement::Discriminator::t_wifi_access_point_settings_ssid_hidden;
    CHECK_TRUE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));

    discriminator = (SettingsElement::Discriminator)-1;
    CHECK_FALSE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
    discriminator = (SettingsElement::Discriminator)12;
    CHECK_FALSE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
    discriminator = (SettingsElement::Discriminator)100;
    CHECK_FALSE(testable.PublicMorozov_ValidateDiscriminator(&discriminator));
}

TEST(LogicSettingsElementTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableSettingsElement testable;
    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_scanner_settings_min_rssi;

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(5, writed);

    CHECK_EQUAL(TvElementType::et_Settings, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(SettingsElement::Discriminator::t_wifi_scanner_settings_min_rssi,
                *((SettingsElement::Discriminator *)&buffer[1]));
}

TEST(LogicSettingsElementTestsGroup, Serialize_just_for_obtain_size) {
    TestableSettingsElement testable;

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(5, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(5, writed);
}

TEST(LogicSettingsElementTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TestableSettingsElement testable;

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicSettingsElementTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_Settings;
    *((SettingsElement::Discriminator *)&buffer[1]) =
        SettingsElement::Discriminator::t_wifi_scanner_settings_per_channel_scan_time_ms;

    TestableSettingsElement testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(4, readed);

    CHECK_EQUAL(SettingsElement::Discriminator::t_wifi_scanner_settings_per_channel_scan_time_ms,
                *testable.PublicMorozov_Get_discriminator());
}

TEST(LogicSettingsElementTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[0] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_Settings;

    TestableSettingsElement testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicSettingsElementTestsGroup, Deserialize_with_wrong_discriminator) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_Settings;
    *((SettingsElement::Discriminator *)&buffer[1]) =
        SettingsElement::Discriminator::t_wifi_station_settings_ssid;

    TestableSettingsElement testable;

    *((SettingsElement::Discriminator *)&buffer[1]) = (SettingsElement::Discriminator)(
        SettingsElement::Discriminator::t_wifi_station_settings_ssid - 1);
    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((SettingsElement::Discriminator *)&buffer[1]) = (SettingsElement::Discriminator)(
        SettingsElement::Discriminator::t_wifi_access_point_settings_ssid_hidden + 1);
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicSettingsElementTestsGroup, ReadValue_with_frendly_format) {
    char display_value[256] = {};
    TestableSettingsElement testable;

    strcpy(settings.wifi_station.ssid, "string_32_0123456789abcdef012345");
    strcpy(settings.wifi_station.password,
           "string_64_0123456789abcdef0123456789abcdef0123456789abcdef012345");
    settings.wifi_station.connect_max_retry_count = -1;
    settings.wifi_station.reconnect_delay_ms = 1234;
    settings.wifi_station.scan_station_rssi_period_ms = 5678;
    settings.wifi_station.max_rssi = 100;
    settings.wifi_station.min_rssi = -120;

    settings.wifi_scanner.per_channel_scan_time_ms = 4219;
    settings.wifi_scanner.max_rssi = 100;
    settings.wifi_scanner.min_rssi = -120;

    settings.wifi_access_point.generation_time_ms = 42;
    settings.wifi_access_point.ssid_hidden = true;

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_ssid;
    testable.PublicMorozov_ReadValue(display_value, true);
    STRCMP_EQUAL("string_32_0123456789abcdef012345", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_password;
    testable.PublicMorozov_ReadValue(display_value, true);
    STRCMP_EQUAL("string_64_0123456789abcdef0123456789abcdef0123456789abcdef012345", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_connect_max_retry_count;
    testable.PublicMorozov_ReadValue(display_value, true);
    STRCMP_EQUAL("infinity", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_reconnect_delay_ms;
    testable.PublicMorozov_ReadValue(display_value, true);
    STRCMP_EQUAL("1234", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_scan_station_rssi_period_ms;
    testable.PublicMorozov_ReadValue(display_value, true);
    STRCMP_EQUAL("5678", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_max_rssi;
    testable.PublicMorozov_ReadValue(display_value, true);
    STRCMP_EQUAL("100", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_min_rssi;
    testable.PublicMorozov_ReadValue(display_value, true);
    STRCMP_EQUAL("-120", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_scanner_settings_per_channel_scan_time_ms;
    testable.PublicMorozov_ReadValue(display_value, true);
    STRCMP_EQUAL("4219", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_scanner_settings_max_rssi;
    testable.PublicMorozov_ReadValue(display_value, true);
    STRCMP_EQUAL("100", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_scanner_settings_min_rssi;
    testable.PublicMorozov_ReadValue(display_value, true);
    STRCMP_EQUAL("-120", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_access_point_settings_generation_time_ms;
    testable.PublicMorozov_ReadValue(display_value, true);
    STRCMP_EQUAL("42", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_access_point_settings_ssid_hidden;
    testable.PublicMorozov_ReadValue(display_value, true);
    STRCMP_EQUAL("true", display_value);
}

TEST(LogicSettingsElementTestsGroup, ReadValue_with_raw_format) {
    char display_value[256] = {};
    TestableSettingsElement testable;

    strcpy(settings.wifi_station.ssid, "string_32_0123456789abcdef012345");
    strcpy(settings.wifi_station.password,
           "string_64_0123456789abcdef0123456789abcdef0123456789abcdef012345");
    settings.wifi_station.connect_max_retry_count = -1;
    settings.wifi_station.reconnect_delay_ms = 1234;
    settings.wifi_station.scan_station_rssi_period_ms = 5678;
    settings.wifi_station.max_rssi = 100;
    settings.wifi_station.min_rssi = -120;

    settings.wifi_scanner.per_channel_scan_time_ms = 4219;
    settings.wifi_scanner.max_rssi = 100;
    settings.wifi_scanner.min_rssi = -120;

    settings.wifi_access_point.generation_time_ms = 42;
    settings.wifi_access_point.ssid_hidden = true;

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_ssid;
    testable.PublicMorozov_ReadValue(display_value, false);
    STRCMP_EQUAL("string_32_0123456789abcdef012345", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_password;
    testable.PublicMorozov_ReadValue(display_value, false);
    STRCMP_EQUAL("string_64_0123456789abcdef0123456789abcdef0123456789abcdef012345", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_connect_max_retry_count;
    testable.PublicMorozov_ReadValue(display_value, false);
    STRCMP_EQUAL("-1", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_reconnect_delay_ms;
    testable.PublicMorozov_ReadValue(display_value, false);
    STRCMP_EQUAL("1234", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_scan_station_rssi_period_ms;
    testable.PublicMorozov_ReadValue(display_value, false);
    STRCMP_EQUAL("5678", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_max_rssi;
    testable.PublicMorozov_ReadValue(display_value, false);
    STRCMP_EQUAL("100", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_min_rssi;
    testable.PublicMorozov_ReadValue(display_value, false);
    STRCMP_EQUAL("-120", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_scanner_settings_per_channel_scan_time_ms;
    testable.PublicMorozov_ReadValue(display_value, false);
    STRCMP_EQUAL("4219", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_scanner_settings_max_rssi;
    testable.PublicMorozov_ReadValue(display_value, false);
    STRCMP_EQUAL("100", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_scanner_settings_min_rssi;
    testable.PublicMorozov_ReadValue(display_value, false);
    STRCMP_EQUAL("-120", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_access_point_settings_generation_time_ms;
    testable.PublicMorozov_ReadValue(display_value, false);
    STRCMP_EQUAL("42", display_value);

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_access_point_settings_ssid_hidden;
    testable.PublicMorozov_ReadValue(display_value, false);
    STRCMP_EQUAL("1", display_value);
}

TEST(LogicSettingsElementTestsGroup, EndEditing_stores_new_ssid_with_max_size) {
    TestableSettingsElement testable;
    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pass");

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_ssid;
    strcpy(testable.PublicMorozov_Get_value(), "string_32_0123456789abcdef012345");
    testable.EndEditing();

    load_settings();
    STRNCMP_EQUAL("string_32_0123456789abcdef012345",
                  settings.wifi_station.ssid,
                  sizeof(settings.wifi_station.ssid));
}

TEST(LogicSettingsElementTestsGroup, EndEditing_stores_new_ssid) {
    TestableSettingsElement testable;
    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pass");

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_ssid;
    strcpy(testable.PublicMorozov_Get_value(), "1234");
    testable.EndEditing();
    load_settings();
    STRNCMP_EQUAL("1234", settings.wifi_station.ssid, sizeof(settings.wifi_station.ssid));

    strcpy(testable.PublicMorozov_Get_value(), "1234567\0028");
    testable.EndEditing();
    load_settings();
    STRNCMP_EQUAL("1234567", settings.wifi_station.ssid, sizeof(settings.wifi_station.ssid));
}

TEST(LogicSettingsElementTestsGroup, EndEditing_stores_empty_ssid) {
    TestableSettingsElement testable;
    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pass");

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_ssid;
    strcpy(testable.PublicMorozov_Get_value(), "");
    testable.EndEditing();

    load_settings();
    STRNCMP_EQUAL("", settings.wifi_station.ssid, sizeof(settings.wifi_station.ssid));
}

TEST(LogicSettingsElementTestsGroup, EndEditing_stores_new_password_with_max_size) {
    TestableSettingsElement testable;
    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pass");

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_password;
    strcpy(testable.PublicMorozov_Get_value(),
           "string_64_0123456789abcdef0123456789abcdef0123456789abcdef012345");
    testable.EndEditing();

    load_settings();
    STRNCMP_EQUAL("string_64_0123456789abcdef0123456789abcdef0123456789abcdef012345",
                  settings.wifi_station.password,
                  sizeof(settings.wifi_station.password));
}

TEST(LogicSettingsElementTestsGroup, EndEditing_stores_password_ssid) {
    TestableSettingsElement testable;
    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pass");

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_password;
    strcpy(testable.PublicMorozov_Get_value(), "1234");
    testable.EndEditing();
    load_settings();
    STRNCMP_EQUAL("1234", settings.wifi_station.password, sizeof(settings.wifi_station.password));

    strcpy(testable.PublicMorozov_Get_value(), "1234567\0028");
    testable.EndEditing();
    load_settings();
    STRNCMP_EQUAL("1234567",
                  settings.wifi_station.password,
                  sizeof(settings.wifi_station.password));
}

TEST(LogicSettingsElementTestsGroup, EndEditing_stores_empty_password) {
    TestableSettingsElement testable;
    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pass");

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_password;
    strcpy(testable.PublicMorozov_Get_value(), "");
    testable.EndEditing();

    load_settings();
    STRNCMP_EQUAL("", settings.wifi_station.password, sizeof(settings.wifi_station.password));
}

TEST(LogicSettingsElementTestsGroup,
     EndEditing_stores_wifi_station_settings_connect_max_retry_count) {
    TestableSettingsElement testable;
    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_connect_max_retry_count;
    strcpy(testable.PublicMorozov_Get_value(), "4219");

    testable.EndEditing();
    load_settings();
    CHECK_EQUAL(4219, settings.wifi_station.connect_max_retry_count);
}

TEST(LogicSettingsElementTestsGroup, EndEditing_stores_wifi_station_settings_reconnect_delay_ms) {
    TestableSettingsElement testable;
    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_reconnect_delay_ms;
    strcpy(testable.PublicMorozov_Get_value(), "4219");

    testable.EndEditing();
    load_settings();
    CHECK_EQUAL(4219, settings.wifi_station.reconnect_delay_ms);
}

TEST(LogicSettingsElementTestsGroup,
     EndEditing_stores_wifi_station_settings_scan_station_rssi_period_ms) {
    TestableSettingsElement testable;
    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_scan_station_rssi_period_ms;
    strcpy(testable.PublicMorozov_Get_value(), "4219");

    testable.EndEditing();
    load_settings();
    CHECK_EQUAL(4219, settings.wifi_station.scan_station_rssi_period_ms);
}

TEST(LogicSettingsElementTestsGroup, EndEditing_stores_wifi_station_settings_max_rssi) {
    TestableSettingsElement testable;
    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_max_rssi;
    strcpy(testable.PublicMorozov_Get_value(), "42");

    testable.EndEditing();
    load_settings();
    CHECK_EQUAL(42, settings.wifi_station.max_rssi);
}

TEST(LogicSettingsElementTestsGroup, EndEditing_stores_wifi_station_settings_min_rssi) {
    TestableSettingsElement testable;
    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_min_rssi;
    strcpy(testable.PublicMorozov_Get_value(), "-42");

    testable.EndEditing();
    load_settings();
    CHECK_EQUAL(-42, settings.wifi_station.min_rssi);
}

TEST(LogicSettingsElementTestsGroup,
     EndEditing_stores_wifi_scanner_settings_per_channel_scan_time_ms) {
    TestableSettingsElement testable;
    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_scanner_settings_per_channel_scan_time_ms;
    strcpy(testable.PublicMorozov_Get_value(), "4219");

    testable.EndEditing();
    load_settings();
    CHECK_EQUAL(4219, settings.wifi_scanner.per_channel_scan_time_ms);
}

TEST(LogicSettingsElementTestsGroup, EndEditing_stores_wifi_scanner_settings_max_rssi) {
    TestableSettingsElement testable;
    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_scanner_settings_max_rssi;
    strcpy(testable.PublicMorozov_Get_value(), "42");

    testable.EndEditing();
    load_settings();
    CHECK_EQUAL(42, settings.wifi_scanner.max_rssi);
}

TEST(LogicSettingsElementTestsGroup, EndEditing_stores_wifi_scanner_settings_min_rssi) {
    TestableSettingsElement testable;
    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_scanner_settings_min_rssi;
    strcpy(testable.PublicMorozov_Get_value(), "-42");

    testable.EndEditing();
    load_settings();
    CHECK_EQUAL(-42, settings.wifi_scanner.min_rssi);
}

TEST(LogicSettingsElementTestsGroup,
     EndEditing_stores_wifi_access_point_settings_generation_time_ms) {
    TestableSettingsElement testable;
    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_access_point_settings_generation_time_ms;
    strcpy(testable.PublicMorozov_Get_value(), "4219");

    testable.EndEditing();
    load_settings();
    CHECK_EQUAL(4219, settings.wifi_access_point.generation_time_ms);
}

TEST(LogicSettingsElementTestsGroup, EndEditing_stores_wifi_access_point_settings_ssid_hidden) {
    TestableSettingsElement testable;
    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_access_point_settings_ssid_hidden;
    strcpy(testable.PublicMorozov_Get_value(), "1");

    testable.EndEditing();
    load_settings();
    CHECK_EQUAL(true, settings.wifi_access_point.ssid_hidden);
}

TEST(LogicSettingsElementTestsGroup, SelectPrior_changing_string_symbols) {
    TestableSettingsElement testable;
    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_ssid;

    strcpy(settings.wifi_station.ssid, "~");

    testable.Change();
    testable.Change();

    CHECK_EQUAL(SettingsElement::EditingPropertyId::cwbepi_Value_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    for (char ch = '~'; ch >= '!'; ch--) {
        CHECK_EQUAL(ch, testable.PublicMorozov_Get_value()[0]);
        CHECK_EQUAL(0, testable.PublicMorozov_Get_value()[1]);
        testable.SelectPrior();
    }
    const char *place_new_char = "\x02";
    STRCMP_EQUAL(place_new_char, testable.PublicMorozov_Get_value());
}

TEST(LogicSettingsElementTestsGroup, SelectPrior_changing_signed_number_symbols) {
    TestableSettingsElement testable;

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_connect_max_retry_count;

    settings.wifi_station.connect_max_retry_count = -1;
    testable.Change();
    testable.Change();

    CHECK_EQUAL(SettingsElement::EditingPropertyId::cwbepi_Value_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    CHECK_EQUAL('-', testable.PublicMorozov_Get_value()[0]);
    CHECK_EQUAL('1', testable.PublicMorozov_Get_value()[1]);
    CHECK_EQUAL(0, testable.PublicMorozov_Get_value()[2]);
    testable.SelectPrior();
    CHECK_EQUAL('\x02', testable.PublicMorozov_Get_value()[0]);
    testable.SelectPrior();

    for (char ch = '9'; ch >= '0'; ch--) {
        CHECK_EQUAL(ch, testable.PublicMorozov_Get_value()[0]);
        CHECK_EQUAL('1', testable.PublicMorozov_Get_value()[1]);
        CHECK_EQUAL(0, testable.PublicMorozov_Get_value()[2]);
        testable.SelectPrior();
    }
    CHECK_EQUAL('-', testable.PublicMorozov_Get_value()[0]);
}

TEST(LogicSettingsElementTestsGroup, SelectPrior_changing_unsigned_number_symbols) {
    TestableSettingsElement testable;

    *testable.PublicMorozov_Get_discriminator() =
        SettingsElement::Discriminator::t_wifi_station_settings_reconnect_delay_ms;

    settings.wifi_station.reconnect_delay_ms = 9;
    testable.Change();
    testable.Change();

    CHECK_EQUAL(SettingsElement::EditingPropertyId::cwbepi_Value_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    for (char ch = '9'; ch >= '0'; ch--) {
        CHECK_EQUAL(ch, testable.PublicMorozov_Get_value()[0]);
        CHECK_EQUAL(0, testable.PublicMorozov_Get_value()[1]);
        testable.SelectPrior();
    }

    CHECK_EQUAL('\x02', testable.PublicMorozov_Get_value()[0]);
}