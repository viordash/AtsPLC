#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Outputs/DecOutput.h"
#include "main/LogicProgram/Outputs/DirectOutput.h"
#include "main/LogicProgram/Outputs/IncOutput.h"
#include "main/LogicProgram/Outputs/ResetOutput.h"
#include "main/LogicProgram/Outputs/SetOutput.h"

TEST_GROUP(LogicDirectOutputTestsGroup){ //
                                         TEST_SETUP(){ mock().disable();
Controller::Start(NULL);
}

TEST_TEARDOWN() {
    Controller::Stop();
    mock().enable();
}
}
;

namespace {
    class TestableDirectOutput : public DirectOutput {
      public:
        TestableDirectOutput() : DirectOutput() {
        }
        virtual ~TestableDirectOutput() {
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        TvElementType PublicMorozov_GetElementType() {
            return GetElementType();
        }
        f_GetValue PublicMorozov_GetValue() {
            return GetValue;
        }
        f_SetValue PublicMorozov_SetValue() {
            return SetValue;
        }
    };
} // namespace

TEST(LogicDirectOutputTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableDirectOutput testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicDirectOutputTestsGroup,
     DoAction_change_state_to_passive__due_incoming_switch_to_passive) {
    TestableDirectOutput testable;
    testable.SetIoAdr(MapIO::V1);

    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());

    CHECK_FALSE_TEXT(testable.DoAction(true, LogicItemState::lisPassive),
                     "no changes are expected to be detected");
    CHECK_FALSE_TEXT(testable.DoAction(false, LogicItemState::lisPassive),
                     "no changes are expected to be detected");
}

TEST(LogicDirectOutputTestsGroup, DoAction_change_state_to_active) {
    TestableDirectOutput testable;
    testable.SetIoAdr(MapIO::V1);

    Controller::SetV1RelativeValue(LogicElement::MinValue);

    CHECK_FALSE(Controller::SampleIOValues());
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::GetV1RelativeValue());
}

TEST(LogicDirectOutputTestsGroup, DoAction_change_state_to_passive) {
    Controller::SetV1RelativeValue(LogicElement::MaxValue);

    TestableDirectOutput testable;
    testable.SetIoAdr(MapIO::V1);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MinValue, Controller::GetV1RelativeValue());
}

TEST(LogicDirectOutputTestsGroup, GetElementType_returns_et_DirectOutput) {
    TestableDirectOutput testable;
    CHECK_EQUAL(TvElementType::et_DirectOutput, testable.PublicMorozov_GetElementType());
}

TEST(LogicDirectOutputTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableDirectOutput testable;
    testable.SetIoAdr(MapIO::O1);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_DirectOutput, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::O1, *((MapIO *)&buffer[1]));
}

TEST(LogicDirectOutputTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_DirectOutput;
    *((MapIO *)&buffer[1]) = MapIO::O2;

    TestableDirectOutput testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);
    CHECK_EQUAL(MapIO::O2, testable.GetIoAdr());
    CHECK(Controller::SetO2RelativeValue == testable.PublicMorozov_SetValue());
    CHECK(Controller::GetO2RelativeValue == testable.PublicMorozov_GetValue());
}

TEST(LogicDirectOutputTestsGroup, GetElementType) {
    TestableDirectOutput testable;
    CHECK_EQUAL(TvElementType::et_DirectOutput, testable.GetElementType());
}

TEST(LogicDirectOutputTestsGroup, TryToCast) {
    DirectOutput directOutput;
    CHECK_TRUE(DirectOutput::TryToCast(&directOutput) == &directOutput);

    SetOutput setOutput;
    CHECK_TRUE(DirectOutput::TryToCast(&setOutput) == NULL);

    ResetOutput resetOutput;
    CHECK_TRUE(DirectOutput::TryToCast(&resetOutput) == NULL);

    IncOutput incOutput;
    CHECK_TRUE(DirectOutput::TryToCast(&incOutput) == NULL);

    DecOutput decOutput;
    CHECK_TRUE(DirectOutput::TryToCast(&decOutput) == NULL);
}