#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Bindings/WiFiApBinding.h"
#include "main/LogicProgram/Bindings/WiFiBinding.h"
#include "main/LogicProgram/Bindings/WiFiStaBinding.h"
#include "main/LogicProgram/Inputs/ComparatorEq.h"
#include "main/LogicProgram/Inputs/ComparatorGE.h"
#include "main/LogicProgram/Inputs/ComparatorGr.h"
#include "main/LogicProgram/Inputs/ComparatorLE.h"
#include "main/LogicProgram/Inputs/ComparatorLs.h"
#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Inputs/InputNO.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

static WiFiService *wifi_service;
TEST_GROUP(LogicWiFiApBindingTestsGroup){
    //
    TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));

mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
mock().expectOneCall("xTaskCreate").ignoreOtherParameters();
wifi_service = new WiFiService();
Controller::Start(NULL, wifi_service);
}

TEST_TEARDOWN() {
    Controller::V1.Unbind();
    Controller::V2.Unbind();
    Controller::V3.Unbind();
    Controller::V4.Unbind();
    Controller::Stop();
    delete wifi_service;
}
}
;

namespace {
    class TestableWiFiApBinding : public WiFiApBinding {
      public:
        TestableWiFiApBinding() : WiFiApBinding() {
        }
        virtual ~TestableWiFiApBinding() {
        }

        const char *GetLabel() {
            return label;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        int *PublicMorozov_Get_editing_property_id() {
            return &editing_property_id;
        }
        bool PublicMorozov_RenderEditedSsid(uint8_t *fb, uint8_t x, uint8_t y) {
            return RenderEditedSsid(fb, x, y);
        }
        uint8_t *PublicMorozov_Get_ssid_size() {
            return &ssid_size;
        }
        uint8_t *PublicMorozov_Get_password_size() {
            return &password_size;
        }
    };
} // namespace

TEST(LogicWiFiApBindingTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicWiFiApBindingTestsGroup,
     DoAction_change_state_to_active_also_switch_variable_binding_to_wifi) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::V1.BindedToWiFi());
}

TEST(LogicWiFiApBindingTestsGroup,
     DoAction_change_state_to_passive_also_switch_variable_binding_to_default) {
    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::V1);
    Controller::V1.BindToWiFi(wifi_service, "test_ssid");
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_FALSE(Controller::V1.BindedToWiFi());
}

TEST(LogicWiFiApBindingTestsGroup, DoAction_change_state_only_by_reason) {
    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::V1.BindedToWiFi());

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::V1.BindedToWiFi());

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_FALSE(Controller::V1.BindedToWiFi());

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_FALSE(Controller::V1.BindedToWiFi());

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::V1.BindedToWiFi());

    CHECK_FALSE(testable.DoAction(true, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::V1.BindedToWiFi());

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_FALSE(Controller::V1.BindedToWiFi());

    CHECK_FALSE(testable.DoAction(true, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_FALSE(Controller::V1.BindedToWiFi());
}

TEST(LogicWiFiApBindingTestsGroup, ssid_changing) {
    TestableWiFiApBinding testable;

    testable.SetSsid("test");
    STRCMP_EQUAL("test", testable.GetSsid());
    CHECK_EQUAL(4, *testable.PublicMorozov_Get_ssid_size());

    testable.SetSsid("ssid_with_size_of_24_chs");
    STRCMP_EQUAL("ssid_with_size_of_24_chs", testable.GetSsid());
    CHECK_EQUAL(24, *testable.PublicMorozov_Get_ssid_size());

    testable.SetSsid("ssid_with_size_of_25_chs0");
    STRCMP_EQUAL("ssid_with_size_of_25_chs", testable.GetSsid());
    CHECK_EQUAL(24, *testable.PublicMorozov_Get_ssid_size());
}

TEST(LogicWiFiApBindingTestsGroup, Change__switching__editing_property_id) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("ssid_with_size_of_24_chs");
    testable.SetPassword("password_16_char");
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding ::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.BeginEditing();
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_ConfigureIOAdr,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    for (int i = 1; i < 24; i++) {
        testable.Change();
        CHECK_TRUE(testable.Editing());
        CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char + i,
                    *testable.PublicMorozov_Get_editing_property_id());
    }

    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Ssid_Last_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    for (int i = 1; i < 16; i++) {
        testable.Change();
        CHECK_TRUE(testable.Editing());
        CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char + i,
                    *testable.PublicMorozov_Get_editing_property_id());
    }

    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Password_Last_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Mac_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    for (int i = 1; i < 12; i++) {
        testable.Change();
        CHECK_TRUE(testable.Editing());
        CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Mac_First_Char + i,
                    *testable.PublicMorozov_Get_editing_property_id());
    }

    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Mac_Last_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.Change();
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    CHECK_EQUAL(24, strlen(testable.GetSsid()));
    CHECK_EQUAL(16, strlen(testable.GetPassword()));
}

TEST(LogicWiFiApBindingTestsGroup, RenderEditedSsid_blink_in_ssid_symbols) {
    volatile uint64_t os_us = 0x80000;
    mock()
        .expectNCalls(24, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("ssid_with_size_of_24_chs");

    int property_id = WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char;
    for (size_t i = 0; i < 24; i++) {
        *testable.PublicMorozov_Get_editing_property_id() = property_id++;
        CHECK_TRUE(
            testable.PublicMorozov_RenderEditedSsid(frame_buffer,
                                                    INCOME_RAIL_WIDTH,
                                                    INCOME_RAIL_TOP + INCOME_RAIL_NETWORK_TOP));
    }
}

TEST(LogicWiFiApBindingTestsGroup, SelectNext_changing_IoAdr) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.BeginEditing();
    testable.Change();
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V2, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
}

TEST(LogicWiFiApBindingTestsGroup, SelectPrior_changing_IoAdr) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.BeginEditing();
    testable.Change();
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V2, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
}

TEST(LogicWiFiApBindingTestsGroup, SelectNext_ssid_0) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("!");
    testable.Change();
    testable.Change();
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    for (char ch = '!'; ch <= '~'; ch++) {
        CHECK_EQUAL(ch, testable.GetSsid()[0]);
        CHECK_EQUAL(0, testable.GetSsid()[1]);
        testable.SelectNext();
    }
    const char *place_new_char = "\x02";
    STRCMP_EQUAL(place_new_char, testable.GetSsid());
}

TEST(LogicWiFiApBindingTestsGroup, SelectPrior_ssid_0) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("~");
    testable.Change();
    testable.Change();
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    for (char ch = '~'; ch >= '!'; ch--) {
        CHECK_EQUAL(ch, testable.GetSsid()[0]);
        CHECK_EQUAL(0, testable.GetSsid()[1]);
        testable.SelectPrior();
    }
    const char *place_new_char = "\x02";
    STRCMP_EQUAL(place_new_char, testable.GetSsid());
}

TEST(LogicWiFiApBindingTestsGroup, PageUp_change_first_ssid_symbol_to_prior_char) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("test");
    testable.Change();
    testable.Change();
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.PageUp();
    STRCMP_EQUAL("sest", testable.GetSsid());
}

TEST(LogicWiFiApBindingTestsGroup, PageUp_change_ssid_symbol_to__new_char) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("test");
    testable.Change();
    testable.Change();
    testable.Change();
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char + 1,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.PageUp();
    const char *place_new_char = "t\x02st";
    STRCMP_EQUAL(place_new_char, testable.GetSsid());
}

TEST(LogicWiFiApBindingTestsGroup, PageDown_change_first_ssid_symbol_to_next_char) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("test");
    testable.Change();
    testable.Change();
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.PageDown();
    STRCMP_EQUAL("uest", testable.GetSsid());
}

TEST(LogicWiFiApBindingTestsGroup, PageDown_change_ssid_symbol_to__new_char) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("test");
    testable.Change();
    testable.Change();
    testable.Change();
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char + 1,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.PageDown();
    const char *place_new_char = "t\x02st";
    STRCMP_EQUAL(place_new_char, testable.GetSsid());
}

TEST(LogicWiFiApBindingTestsGroup, ssid_trimmed_after_editing) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("ssid_with_size_of_24_chs");
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding ::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.BeginEditing();
    testable.Change();
    testable.Change();
    testable.Change();
    testable.Change();
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Ssid_First_Char + 3,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.PageDown();
    const char *place_new_char = "ssi\x02_with_size_of_24_chs";
    STRCMP_EQUAL(place_new_char, testable.GetSsid());

    testable.Option();
    testable.Option();
    testable.Option();
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    CHECK_EQUAL(3, strlen(testable.GetSsid()));
}

TEST(LogicWiFiApBindingTestsGroup, password_trimmed_after_editing) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetPassword("password_16_char");
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding ::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.BeginEditing();
    testable.Change();
    testable.Change();
    testable.Option();
    testable.Change();
    testable.Change();
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_Password_First_Char + 3,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.PageDown();
    const char *place_new_char = "pas\x02word_16_char";
    STRCMP_EQUAL(place_new_char, testable.GetPassword());

    testable.Option();
    testable.Option();
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiApBinding::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    CHECK_EQUAL(3, strlen(testable.GetPassword()));
}

TEST(LogicWiFiApBindingTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::V2);
    testable.SetSsid("ssid");
    testable.SetPassword("secret");
    testable.SetMac("0123456789AB");

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(57, writed);

    CHECK_EQUAL(TvElementType::et_WiFiApBinding, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::V2, *((MapIO *)&buffer[1]));
    STRCMP_EQUAL("ssid", (char *)&buffer[2]);
    STRCMP_EQUAL("secret", (char *)&buffer[27]);
    STRCMP_EQUAL("0123456789AB", (char *)&buffer[44]);
}

TEST(LogicWiFiApBindingTestsGroup, Serialize_just_for_obtain_size) {
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("ssid");
    testable.SetPassword("secret");
    testable.SetMac("0123456789AB");

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(57, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(57, writed);
}

TEST(LogicWiFiApBindingTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TestableWiFiApBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("ssid");
    testable.SetPassword("secret");
    testable.SetMac("0123456789AB");

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicWiFiApBindingTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiApBinding;
    *((MapIO *)&buffer[1]) = MapIO::V3;
    strcpy((char *)&buffer[2], "test_ssid");
    strcpy((char *)&buffer[27], "test_secret");
    strcpy((char *)&buffer[44], "0123456789AB");

    TestableWiFiApBinding testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(56, readed);

    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    CHECK(&Controller::V3 == testable.Input);
    STRCMP_EQUAL("test_ssid", testable.GetSsid());
    STRCMP_EQUAL("test_secret", testable.GetPassword());
    STRCMP_EQUAL("0123456789AB", testable.GetMac());
}

TEST(LogicWiFiApBindingTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[0] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiApBinding;

    TestableWiFiApBinding testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicWiFiApBindingTestsGroup, Deserialize_with_wrong_io_adr_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiApBinding;
    strcpy((char *)&buffer[2], "test_ssid");
    strcpy((char *)&buffer[27], "test_secret");
    strcpy((char *)&buffer[44], "0123456789AB");

    TestableWiFiApBinding testable;

    *((MapIO *)&buffer[1]) = (MapIO)(MapIO::DI - 1);
    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[1]) = (MapIO)(MapIO::V4 + 1);
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[1]) = MapIO::DI;
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(56, readed);
}

TEST(LogicWiFiApBindingTestsGroup, Deserialize_with_wrong_ssid_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiApBinding;
    *((MapIO *)&buffer[1]) = MapIO::V3;
    buffer[2] = 0;
    strcpy((char *)&buffer[27], "test_secret");
    strcpy((char *)&buffer[44], "0123456789AB");

    TestableWiFiApBinding testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    strcpy((char *)&buffer[2], "");
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    strcpy((char *)&buffer[2], "ssid");
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(56, readed);
}

TEST(LogicWiFiApBindingTestsGroup, Deserialize_with_wrong_password_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiApBinding;
    *((MapIO *)&buffer[1]) = MapIO::V3;
    strcpy((char *)&buffer[2], "ssid");
    buffer[27] = 0;
    strcpy((char *)&buffer[44], "0123456789AB");

    TestableWiFiApBinding testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    strcpy((char *)&buffer[27], "");
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    strcpy((char *)&buffer[27], "test_secret");
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(56, readed);
}

TEST(LogicWiFiApBindingTestsGroup, Deserialize_with_wrong_mac_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiApBinding;
    *((MapIO *)&buffer[1]) = MapIO::V3;
    strcpy((char *)&buffer[2], "ssid");
    strcpy((char *)&buffer[27], "test_secret");
    buffer[44] = 0;

    TestableWiFiApBinding testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    strcpy((char *)&buffer[44], "");
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    strcpy((char *)&buffer[44], "0123456789AB");
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(56, readed);
}

TEST(LogicWiFiApBindingTestsGroup, GetElementType) {
    TestableWiFiApBinding testable;
    CHECK_EQUAL(TvElementType::et_WiFiApBinding, testable.GetElementType());
}

TEST(LogicWiFiApBindingTestsGroup, TryToCast) {
    InputNC inputNC;
    CHECK_TRUE(WiFiApBinding::TryToCast(&inputNC) == NULL);

    InputNO inputNO;
    CHECK_TRUE(WiFiApBinding::TryToCast(&inputNO) == NULL);

    ComparatorEq comparatorEq;
    CHECK_TRUE(WiFiApBinding::TryToCast(&comparatorEq) == NULL);

    ComparatorGE comparatorGE;
    CHECK_TRUE(WiFiApBinding::TryToCast(&comparatorGE) == NULL);

    ComparatorGr comparatorGr;
    CHECK_TRUE(WiFiApBinding::TryToCast(&comparatorGr) == NULL);

    ComparatorLE comparatorLE;
    CHECK_TRUE(WiFiApBinding::TryToCast(&comparatorLE) == NULL);

    ComparatorLs comparatorLs;
    CHECK_TRUE(WiFiApBinding::TryToCast(&comparatorLs) == NULL);

    WiFiStaBinding wiFiStaBinding;
    CHECK_TRUE(WiFiApBinding::TryToCast(&wiFiStaBinding) == NULL);

    WiFiApBinding wiFiBinding;
    CHECK_TRUE(WiFiApBinding::TryToCast(&wiFiBinding) == &wiFiBinding);
}

TEST(LogicWiFiApBindingTestsGroup, SetPassword) {
    TestableWiFiApBinding testable;
    testable.SetPassword("secret");
    STRCMP_EQUAL("secret", testable.GetPassword());
}

TEST(LogicWiFiApBindingTestsGroup, SetMac) {
    TestableWiFiApBinding testable;
    testable.SetMac("");
    STRCMP_EQUAL("************", testable.GetMac());
    testable.SetMac("0");
    STRCMP_EQUAL("0***********", testable.GetMac());
    testable.SetMac("01234");
    STRCMP_EQUAL("01234*******", testable.GetMac());
    testable.SetMac("0123456789");
    STRCMP_EQUAL("0123456789**", testable.GetMac());
    testable.SetMac("0123456789AB");
    STRCMP_EQUAL("0123456789AB", testable.GetMac());
    testable.SetMac("0123456789ABCDEF");
    STRCMP_EQUAL("0123456789AB", testable.GetMac());
}