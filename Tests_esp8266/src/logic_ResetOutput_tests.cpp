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

TEST_GROUP(LogicResetOutputTestsGroup){ //
                                        TEST_SETUP(){ mock().disable();
Controller::Start(NULL, NULL);
}

TEST_TEARDOWN() {
    Controller::Stop();
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

    Controller::V1.WriteValue(LogicElement::MaxValue);
    Controller::V1.CommitChanges();

    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    Controller::CommitChanges();
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    Controller::V1.ReadValue();
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MinValue, Controller::V1.ReadValue());

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    Controller::CommitChanges();
    CHECK_EQUAL(LogicElement::MinValue, Controller::V1.ReadValue());
}

TEST(LogicResetOutputTestsGroup, DoAction_change_state_to_passive) {
    Controller::V1.WriteValue(LogicElement::MaxValue);
    Controller::V1.CommitChanges();

    TestableResetOutput testable;
    testable.SetIoAdr(MapIO::V1);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    Controller::V1.CommitChanges();
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
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
    *((MapIO *)&buffer[1]) = MapIO::O2;

    TestableResetOutput testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);
    CHECK_EQUAL(MapIO::O2, testable.GetIoAdr());
    CHECK(&Controller::O2 == testable.Output);
    CHECK(&Controller::O2 == testable.Input);
}

TEST(LogicResetOutputTestsGroup, GetElementType) {
    TestableResetOutput testable;
    CHECK_EQUAL(TvElementType::et_ResetOutput, testable.GetElementType());
}

TEST(LogicResetOutputTestsGroup, TryToCast) {
    DirectOutput directOutput;
    CHECK_TRUE(ResetOutput::TryToCast(&directOutput) == NULL);

    SetOutput setOutput;
    CHECK_TRUE(ResetOutput::TryToCast(&setOutput) == NULL);

    ResetOutput resetOutput;
    CHECK_TRUE(ResetOutput::TryToCast(&resetOutput) == &resetOutput);

    IncOutput incOutput;
    CHECK_TRUE(ResetOutput::TryToCast(&incOutput) == NULL);

    DecOutput decOutput;
    CHECK_TRUE(ResetOutput::TryToCast(&decOutput) == NULL);
}