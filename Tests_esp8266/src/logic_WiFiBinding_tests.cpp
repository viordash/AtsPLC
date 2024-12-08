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

TEST_GROUP(LogicWiFiBindingTestsGroup){ //
                                        TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));

mock().disable();
Controller::Start(NULL);
}

TEST_TEARDOWN() {
    Controller::Stop();
    mock().enable();
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