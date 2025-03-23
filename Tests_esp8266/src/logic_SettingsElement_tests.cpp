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
        bool PublicMorozov_ValidateDiscriminator(Discriminator *discriminator) {
            return ValidateDiscriminator(discriminator);
        }

        Discriminator *PublicMorozov_Get_discriminator() {
            return &discriminator;
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
