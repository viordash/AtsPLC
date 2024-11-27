#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/ComparatorEq.h"
#include "main/LogicProgram/Inputs/ComparatorGE.h"
#include "main/LogicProgram/Inputs/ComparatorGr.h"
#include "main/LogicProgram/Inputs/ComparatorLE.h"
#include "main/LogicProgram/Inputs/ComparatorLs.h"
#include "main/LogicProgram/Inputs/InputNO.h"
#include "main/LogicProgram/Inputs/WiFiInput.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicWiFiInputTestsGroup){ //
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
    class TestableWiFiInput : public WiFiInput {
      public:
        TestableWiFiInput() : WiFiInput() {
        }
        virtual ~TestableWiFiInput() {
        }

        const char *GetLabel() {
            return label;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        f_GetValue PublicMorozov_GetValue() {
            return GetValue;
        }
    };
} // namespace

TEST(LogicWiFiInputTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableWiFiInput testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicWiFiInputTestsGroup, DoAction_change_state_to_passive__due_incoming_switch_to_passive) {
    TestableWiFiInput testable;
    testable.SetIoAdr(MapIO::V1);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;
    Controller::GetIOValues().V1 = LogicElement::MaxValue;

    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    CHECK_FALSE(Controller::SampleIOValues());
    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());

    CHECK_FALSE_TEXT(testable.DoAction(true, LogicItemState::lisPassive),
                     "no changes are expected to be detected");
    CHECK_FALSE_TEXT(testable.DoAction(false, LogicItemState::lisPassive),
                     "no changes are expected to be detected");
}

TEST(LogicWiFiInputTestsGroup, DoAction_change_state_to_active) {
    Controller::GetIOValues().V1 = LogicElement::MaxValue;

    TestableWiFiInput testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
}

IGNORE_TEST(LogicWiFiInputTestsGroup, DoAction_change_state_to_passive) {
    Controller::GetIOValues().V1 = LogicElement::MinValue;

    TestableWiFiInput testable;
    testable.SetIoAdr(MapIO::V1);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicWiFiInputTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableWiFiInput testable;
    testable.SetIoAdr(MapIO::V2);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_WiFiInput, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::V2, *((MapIO *)&buffer[1]));
}

TEST(LogicWiFiInputTestsGroup, Serialize_just_for_obtain_size) {
    TestableWiFiInput testable;
    testable.SetIoAdr(MapIO::V1);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(2, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(2, writed);
}

TEST(LogicWiFiInputTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TestableWiFiInput testable;
    testable.SetIoAdr(MapIO::V1);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicWiFiInputTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiInput;
    *((MapIO *)&buffer[1]) = MapIO::V3;

    TestableWiFiInput testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);

    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    CHECK(Controller::GetV3RelativeValue == testable.PublicMorozov_GetValue());
}

TEST(LogicWiFiInputTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[0] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiInput;

    TestableWiFiInput testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

IGNORE_TEST(LogicWiFiInputTestsGroup, Deserialize_with_wrong_io_adr_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiInput;

    TestableWiFiInput testable;

    *((MapIO *)&buffer[1]) = (MapIO)(MapIO::V1 - 1);
    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[1]) = (MapIO)(MapIO::V4 + 1);
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[1]) = MapIO::V1;
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);
}

TEST(LogicWiFiInputTestsGroup, GetElementType) {
    TestableWiFiInput testable;
    CHECK_EQUAL(TvElementType::et_WiFiInput, testable.GetElementType());
}

TEST(LogicWiFiInputTestsGroup, TryToCast) {
    WiFiInput inputNC;
    CHECK_TRUE(WiFiInput::TryToCast(&inputNC) == &inputNC);

    InputNO inputNO;
    CHECK_TRUE(WiFiInput::TryToCast(&inputNO) == NULL);

    ComparatorEq comparatorEq;
    CHECK_TRUE(WiFiInput::TryToCast(&comparatorEq) == NULL);

    ComparatorGE comparatorGE;
    CHECK_TRUE(WiFiInput::TryToCast(&comparatorGE) == NULL);

    ComparatorGr comparatorGr;
    CHECK_TRUE(WiFiInput::TryToCast(&comparatorGr) == NULL);

    ComparatorLE comparatorLE;
    CHECK_TRUE(WiFiInput::TryToCast(&comparatorLE) == NULL);

    ComparatorLs comparatorLs;
    CHECK_TRUE(WiFiInput::TryToCast(&comparatorLs) == NULL);
}
