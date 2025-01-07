#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Bindings/WiFiBinding.h"
#include "main/LogicProgram/Inputs/ComparatorEq.h"
#include "main/LogicProgram/Inputs/ComparatorGE.h"
#include "main/LogicProgram/Inputs/ComparatorGr.h"
#include "main/LogicProgram/Inputs/ComparatorLE.h"
#include "main/LogicProgram/Inputs/ComparatorLs.h"
#include "main/LogicProgram/Inputs/InputNO.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

WiFiService *wifi_service;
TEST_GROUP(LogicWiFiBindingTestsGroup){ //
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
    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    TestableWiFiBinding testable;
    testable.SetIoAdr(MapIO::V1);
    Controller::V1.BindToWiFi(wifi_service, "test_ssid");
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_FALSE(Controller::V1.BindedToWiFi());
}

TEST(LogicWiFiBindingTestsGroup, DoAction_change_state_only_by_reason) {
    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer).expectNCalls(2, "xEventGroupSetBits").ignoreOtherParameters();

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
    testable.SetSsid("ssid_with_size_of_24_chs");
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiBinding ::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.BeginEditing();
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_ConfigureIOAdr,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    for (int i = 1; i < 24; i++) {
        testable.Change();
        CHECK_TRUE(testable.Editing());
        CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_First_Char + i,
                    *testable.PublicMorozov_Get_editing_property_id());
    }

    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_Ssid_Last_Char,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.Change();
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(WiFiBinding::EditingPropertyId::wbepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    CHECK_EQUAL(24, strlen(testable.GetSsid()));
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