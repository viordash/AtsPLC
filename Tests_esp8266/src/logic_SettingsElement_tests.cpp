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
        bool PublicMorozov_ValidateValue(Discriminator *discriminator, Value *value) {
            return ValidateValue(discriminator, value);
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

TEST(LogicSettingsElementTestsGroup, ValidateValue_string) {
    TestableSettingsElement testable;
    SettingsElement::Discriminator discriminator =
        SettingsElement::Discriminator::t_wifi_station_settings_ssid;

    SettingsElement::Value value;
    strcpy(value.string_value, "t");
    CHECK_TRUE(testable.PublicMorozov_ValidateValue(&discriminator, &value));
    strcpy(value.string_value, "string_64_0123456789abcdef0123456789abcdef0123456789abcdef012345");
    CHECK_TRUE(testable.PublicMorozov_ValidateValue(&discriminator, &value));

    strcpy(value.string_value, "string_65_0123456789abcdef0123456789abcdef0123456789abcdef0123456");
    CHECK_FALSE(testable.PublicMorozov_ValidateValue(&discriminator, &value));
    strcpy(value.string_value,
           "string_66_0123456789abcdef0123456789abcdef0123456789abcdef01234567");
    CHECK_FALSE(testable.PublicMorozov_ValidateValue(&discriminator, &value));
    strcpy(value.string_value, "");
    CHECK_FALSE(testable.PublicMorozov_ValidateValue(&discriminator, &value));
}

TEST(LogicSettingsElementTestsGroup, ValidateValue_bool) {
    TestableSettingsElement testable;
    SettingsElement::Discriminator discriminator =
        SettingsElement::Discriminator::t_wifi_access_point_settings_ssid_hidden;

    SettingsElement::Value value;
    value.bool_value = true;
    CHECK_TRUE(testable.PublicMorozov_ValidateValue(&discriminator, &value));
    value.bool_value = false;
    CHECK_TRUE(testable.PublicMorozov_ValidateValue(&discriminator, &value));

    value.int32_value = 2;
    CHECK_FALSE(testable.PublicMorozov_ValidateValue(&discriminator, &value));
    value.int32_value = -1;
    CHECK_FALSE(testable.PublicMorozov_ValidateValue(&discriminator, &value));
}

// TEST(LogicSettingsElementTestsGroup, Serialize) {
//     uint8_t buffer[256] = {};
//     TestableSettingsElement testable;

//     size_t writed = testable.Serialize(buffer, sizeof(buffer));
//     CHECK_EQUAL(57, writed);

//     CHECK_EQUAL(TvElementType::et_SettingsElement, *((TvElementType *)&buffer[0]));
//     CHECK_EQUAL(MapIO::V2, *((MapIO *)&buffer[1]));
//     STRCMP_EQUAL("ssid", (char *)&buffer[2]);
//     STRCMP_EQUAL("secret", (char *)&buffer[27]);
//     STRCMP_EQUAL("0123456789AB", (char *)&buffer[44]);
// }

// TEST(LogicSettingsElementTestsGroup, Serialize_just_for_obtain_size) {
//     TestableSettingsElement testable;
//     testable.SetIoAdr(MapIO::DI);
//     testable.SetSsid("ssid");
//     testable.SetPassword("secret");
//     testable.SetMac("0123456789AB");

//     size_t writed = testable.Serialize(NULL, SIZE_MAX);
//     CHECK_EQUAL(57, writed);

//     writed = testable.Serialize(NULL, 0);
//     CHECK_EQUAL(57, writed);
// }

// TEST(LogicSettingsElementTestsGroup, Serialize_to_small_buffer_return_zero) {
//     uint8_t buffer[1] = {};
//     TestableSettingsElement testable;
//     testable.SetIoAdr(MapIO::DI);
//     testable.SetSsid("ssid");
//     testable.SetPassword("secret");
//     testable.SetMac("0123456789AB");

//     size_t writed = testable.Serialize(buffer, sizeof(buffer));
//     CHECK_EQUAL(0, writed);
// }

// TEST(LogicSettingsElementTestsGroup, Deserialize) {
//     uint8_t buffer[256] = {};
//     *((TvElementType *)&buffer[0]) = TvElementType::et_SettingsElement;
//     *((MapIO *)&buffer[1]) = MapIO::V3;
//     strcpy((char *)&buffer[2], "test_ssid");
//     strcpy((char *)&buffer[27], "test_secret");
//     strcpy((char *)&buffer[44], "0123456789AB");

//     TestableSettingsElement testable;

//     size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
//     CHECK_EQUAL(56, readed);

//     CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
//     CHECK(&Controller::V3 == testable.Input);
//     STRCMP_EQUAL("test_ssid", testable.GetSsid());
//     STRCMP_EQUAL("test_secret", testable.GetPassword());
//     STRCMP_EQUAL("0123456789AB", testable.GetMac());
// }

// TEST(LogicSettingsElementTestsGroup, Deserialize_with_small_buffer_return_zero) {
//     uint8_t buffer[0] = {};
//     *((TvElementType *)&buffer[0]) = TvElementType::et_SettingsElement;

//     TestableSettingsElement testable;

//     size_t readed = testable.Deserialize(buffer, sizeof(buffer));
//     CHECK_EQUAL(0, readed);
// }

// TEST(LogicSettingsElementTestsGroup, Deserialize_with_wrong_io_adr_return_zero) {
//     uint8_t buffer[256] = {};
//     *((TvElementType *)&buffer[0]) = TvElementType::et_SettingsElement;
//     strcpy((char *)&buffer[2], "test_ssid");
//     strcpy((char *)&buffer[27], "test_secret");
//     strcpy((char *)&buffer[44], "0123456789AB");

//     TestableSettingsElement testable;

//     *((MapIO *)&buffer[1]) = (MapIO)(MapIO::DI - 1);
//     size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
//     CHECK_EQUAL(0, readed);

//     *((MapIO *)&buffer[1]) = (MapIO)(MapIO::V4 + 1);
//     readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
//     CHECK_EQUAL(0, readed);

//     *((MapIO *)&buffer[1]) = MapIO::DI;
//     readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
//     CHECK_EQUAL(56, readed);
// }

// TEST(LogicSettingsElementTestsGroup, Deserialize_with_wrong_ssid_return_zero) {
//     uint8_t buffer[256] = {};
//     *((TvElementType *)&buffer[0]) = TvElementType::et_SettingsElement;
//     *((MapIO *)&buffer[1]) = MapIO::V3;
//     buffer[2] = 0;
//     strcpy((char *)&buffer[27], "test_secret");
//     strcpy((char *)&buffer[44], "0123456789AB");

//     TestableSettingsElement testable;

//     size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
//     CHECK_EQUAL(0, readed);

//     strcpy((char *)&buffer[2], "");
//     readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
//     CHECK_EQUAL(0, readed);

//     strcpy((char *)&buffer[2], "ssid");
//     readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
//     CHECK_EQUAL(56, readed);
// }