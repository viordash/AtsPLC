#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Outputs/ResetOutput.h"

TEST_GROUP(LogicResetOutputTestsGroup){ //
                                        TEST_SETUP(){ mock().disable();
Controller::Stop();
}

TEST_TEARDOWN() {
    mock().enable();
}
}
;

namespace {
    class TestableResetOutput : public ResetOutput {
      public:
        TestableResetOutput() : ResetOutput() {
        }
        virtual ~TestableResetOutput() {
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        TvElementType PublicMorozov_GetElementType() {
            return GetElementType();
        }
    };
} // namespace

TEST(LogicResetOutputTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableResetOutput testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicResetOutputTestsGroup, DoAction_change_state_to_passive__due_incoming_switch_to_passive) {
    TestableResetOutput testable;
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

TEST(LogicResetOutputTestsGroup, DoAction_change_state_to_active__and_second_call_does_nothing) {
    TestableResetOutput testable;
    testable.SetIoAdr(MapIO::V1);

    Controller::SetV1RelativeValue(LogicElement::MaxValue);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MinValue, Controller::GetV1RelativeValue());

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicElement::MinValue, Controller::GetV1RelativeValue());
}

TEST(LogicResetOutputTestsGroup, DoAction_change_state_to_passive) {
    Controller::SetV1RelativeValue(LogicElement::MaxValue);

    TestableResetOutput testable;
    testable.SetIoAdr(MapIO::V1);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::GetV1RelativeValue());
}

TEST(LogicResetOutputTestsGroup, GetElementType_returns_et_ResetOutput) {
    TestableResetOutput testable;
    CHECK_EQUAL(TvElementType::et_ResetOutput, testable.PublicMorozov_GetElementType());
}

TEST(LogicResetOutputTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableResetOutput testable;
    testable.SetIoAdr(MapIO::O1);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_ResetOutput, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::O1, *((MapIO *)&buffer[1]));
}

TEST(LogicResetOutputTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_ResetOutput;

    TestableResetOutput testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);
}

TEST(LogicResetOutputTestsGroup, GetElementType) {
    TestableResetOutput testable;
    CHECK_EQUAL(TvElementType::et_ResetOutput, testable.GetElementType());
}