#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Bindings/WiFiBinding.h"
#include "main/LogicProgram/Bindings/WiFiStaBinding.h"
#include "main/LogicProgram/Inputs/ComparatorEq.h"
#include "main/LogicProgram/Inputs/ComparatorGE.h"
#include "main/LogicProgram/Inputs/ComparatorGr.h"
#include "main/LogicProgram/Inputs/ComparatorLE.h"
#include "main/LogicProgram/Inputs/ComparatorLs.h"
#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Inputs/InputNO.h"

static uint8_t frame_buffer[DISPLAY_HEIGHT_IN_BYTES * DISPLAY_WIDTH] = {};

static WiFiService *wifi_service;
TEST_GROUP(LogicWiFiBindingTestsGroup){ //
                                        TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));

mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
mock().expectOneCall("xTaskCreate").ignoreOtherParameters();
wifi_service = new WiFiService();
Controller::Start(NULL, wifi_service, NULL, NULL);
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
    class TestableWiFiBinding : public WiFiBinding {
      public:
        TestableWiFiBinding() : WiFiBinding() {
        }
        virtual ~TestableWiFiBinding() {
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
    };
} // namespace

TEST(LogicWiFiBindingTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicWiFiBindingTestsGroup,
     DoAction_change_state_to_active_also_switch_variable_binding_to_wifi) {
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::V1.BindedToWiFi());
}

TEST(LogicWiFiBindingTestsGroup,
     DoAction_change_state_to_passive_also_switch_variable_binding_to_default) {

    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::V1);
    Controller::V1.BindToInsecureWiFi(wifi_service, "test_ssid");
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_FALSE(Controller::V1.BindedToWiFi());
}

TEST(LogicWiFiBindingTestsGroup, DoAction_change_state_only_by_reason) {
    TestableWiFiBinding testable;
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

TEST(LogicWiFiBindingTestsGroup, ssid_changing) {
    TestableWiFiBinding testable;

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

TEST(LogicWiFiBindingTestsGroup, Change__switching__editing_property_id) {
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("");
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiBinding ::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.BeginEditing();
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();

    for (int i = 0; i < 23; i++) {
        CHECK_TRUE(testable.Editing());
        CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char + i,
                    *testable.PublicMorozov_Get_editing_property_id());
        testable.SelectNext();
        testable.Change();
    }

    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_Last_Char,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.SelectNext();
    testable.Change();

    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    CHECK_EQUAL(24, strlen(testable.GetSsid()));
    STRCMP_EQUAL("!!!!!!!!!!!!!!!!!!!!!!!!", testable.GetSsid());
}

TEST(LogicWiFiBindingTestsGroup, Ssid_with_partial_length) {
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("");
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiBinding ::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.BeginEditing();
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();

    for (int i = 0; i < 5; i++) {
        CHECK_TRUE(testable.Editing());
        CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char + i,
                    *testable.PublicMorozov_Get_editing_property_id());
        testable.SelectNext();
        testable.Change();
    }

    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char + 5,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();

    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    CHECK_EQUAL(5, strlen(testable.GetSsid()));
    STRCMP_EQUAL("!!!!!", testable.GetSsid());
}

TEST(LogicWiFiBindingTestsGroup, RenderEditedSsid_blink_in_ssid_symbols) {
    volatile uint64_t os_us = 0x80000;
    mock()
        .expectNCalls(24, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("ssid_with_size_of_24_chs");

    int property_id = WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char;
    for (size_t i = 0; i < 24; i++) {
        *testable.PublicMorozov_Get_editing_property_id() = property_id++;
        CHECK_TRUE(
            testable.PublicMorozov_RenderEditedSsid(frame_buffer,
                                                    INCOME_RAIL_WIDTH,
                                                    INCOME_RAIL_TOP + INCOME_RAIL_NETWORK_TOP));
    }
}

TEST(LogicWiFiBindingTestsGroup, SelectNext_changing_IoAdr) {
    TestableWiFiBinding testable;
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

TEST(LogicWiFiBindingTestsGroup, SelectPrior_changing_IoAdr) {
    TestableWiFiBinding testable;
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

TEST(LogicWiFiBindingTestsGroup, SelectNext_ssid_0) {
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("!");
    testable.Change();
    testable.Change();
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    for (char ch = '!'; ch <= '~'; ch++) {
        CHECK_EQUAL(ch, testable.GetSsid()[0]);
        CHECK_EQUAL(0, testable.GetSsid()[1]);
        testable.SelectNext();
    }
    const char *place_new_char = "\x02";
    STRCMP_EQUAL(place_new_char, testable.GetSsid());
}

TEST(LogicWiFiBindingTestsGroup, SelectPrior_ssid_0) {
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("~");
    testable.Change();
    testable.Change();
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    for (char ch = '~'; ch >= '!'; ch--) {
        CHECK_EQUAL(ch, testable.GetSsid()[0]);
        CHECK_EQUAL(0, testable.GetSsid()[1]);
        testable.SelectPrior();
    }
    const char *place_new_char = "\x02";
    STRCMP_EQUAL(place_new_char, testable.GetSsid());
}

TEST(LogicWiFiBindingTestsGroup, PageUp_change_first_ssid_symbol_to_prior_char) {
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("test");
    testable.Change();
    testable.Change();
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.PageUp();
    STRCMP_EQUAL("sest", testable.GetSsid());
}

TEST(LogicWiFiBindingTestsGroup, PageUp_change_ssid_symbol_to__new_char) {
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("test");
    testable.Change();
    testable.Change();
    testable.Change();
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char + 1,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.PageUp();
    const char *place_new_char = "t\x02st";
    STRCMP_EQUAL(place_new_char, testable.GetSsid());
}

TEST(LogicWiFiBindingTestsGroup, PageDown_change_first_ssid_symbol_to_next_char) {
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("test");
    testable.Change();
    testable.Change();
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.PageDown();
    STRCMP_EQUAL("uest", testable.GetSsid());
}

TEST(LogicWiFiBindingTestsGroup, PageDown_change_ssid_symbol_to__new_char) {
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("test");
    testable.Change();
    testable.Change();
    testable.Change();
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char + 1,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.PageDown();
    const char *place_new_char = "t\x02st";
    STRCMP_EQUAL(place_new_char, testable.GetSsid());
}

TEST(LogicWiFiBindingTestsGroup, ssid_trimmed_after_editing) {
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.SetSsid("ssid_with_size_of_24_chs");
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiBinding ::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.BeginEditing();
    testable.Change();
    testable.Change();
    testable.Change();
    testable.Change();
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char + 3,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.PageDown();
    const char *place_new_char = "ssi\x02_with_size_of_24_chs";
    STRCMP_EQUAL(place_new_char, testable.GetSsid());

    testable.Change();
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    CHECK_EQUAL(3, strlen(testable.GetSsid()));
}

TEST(LogicWiFiBindingTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::V2);
    testable.SetSsid("ssid");

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(27, writed);

    CHECK_EQUAL(TvElementType::et_WiFiBinding, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::V2, *((MapIO *)&buffer[1]));
    STRCMP_EQUAL("ssid", (char *)&buffer[2]);
}

TEST(LogicWiFiBindingTestsGroup, Serialize_just_for_obtain_size) {
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::DI);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(27, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(27, writed);
}

TEST(LogicWiFiBindingTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::DI);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicWiFiBindingTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiBinding;
    *((MapIO *)&buffer[1]) = MapIO::V3;
    strcpy((char *)&buffer[2], "test_ssid");

    TestableWiFiBinding testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(26, readed);

    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    CHECK(&Controller::V3 == testable.Input);
    STRCMP_EQUAL("test_ssid", testable.GetSsid());
}

TEST(LogicWiFiBindingTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiBinding;

    TestableWiFiBinding testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicWiFiBindingTestsGroup, Deserialize_with_wrong_io_adr_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiBinding;
    strcpy((char *)&buffer[2], "test_ssid");

    TestableWiFiBinding testable;

    *((MapIO *)&buffer[1]) = (MapIO)(MapIO::DI - 1);
    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[1]) = (MapIO)(MapIO::V4 + 1);
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[1]) = MapIO::DI;
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(26, readed);
}

TEST(LogicWiFiBindingTestsGroup, Deserialize_with_wrong_ssid_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiBinding;
    *((MapIO *)&buffer[1]) = MapIO::V3;
    buffer[2] = 0;

    TestableWiFiBinding testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    strcpy((char *)&buffer[2], "");
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    strcpy((char *)&buffer[2], "ssid");
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(26, readed);
}

TEST(LogicWiFiBindingTestsGroup, GetElementType) {
    TestableWiFiBinding testable;
    CHECK_EQUAL(TvElementType::et_WiFiBinding, testable.GetElementType());
}

TEST(LogicWiFiBindingTestsGroup, TryToCast) {
    InputNC inputNC;
    CHECK_TRUE(WiFiBinding::TryToCast(&inputNC) == NULL);

    InputNO inputNO;
    CHECK_TRUE(WiFiBinding::TryToCast(&inputNO) == NULL);

    ComparatorEq comparatorEq;
    CHECK_TRUE(WiFiBinding::TryToCast(&comparatorEq) == NULL);

    ComparatorGE comparatorGE;
    CHECK_TRUE(WiFiBinding::TryToCast(&comparatorGE) == NULL);

    ComparatorGr comparatorGr;
    CHECK_TRUE(WiFiBinding::TryToCast(&comparatorGr) == NULL);

    ComparatorLE comparatorLE;
    CHECK_TRUE(WiFiBinding::TryToCast(&comparatorLE) == NULL);

    ComparatorLs comparatorLs;
    CHECK_TRUE(WiFiBinding::TryToCast(&comparatorLs) == NULL);

    WiFiStaBinding wiFiStaBinding;
    CHECK_TRUE(WiFiBinding::TryToCast(&wiFiStaBinding) == NULL);

    WiFiBinding wiFiBinding;
    CHECK_TRUE(WiFiBinding::TryToCast(&wiFiBinding) == &wiFiBinding);
}