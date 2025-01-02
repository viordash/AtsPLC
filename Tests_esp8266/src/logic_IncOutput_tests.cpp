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

TEST_GROUP(LogicIncOutputTestsGroup){ //
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
    class TestableIncOutput : public IncOutput {
      public:
        TestableIncOutput() : IncOutput() {
        }
        virtual ~TestableIncOutput() {
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        TvElementType PublicMorozov_GetElementType() {
            return GetElementType();
        }
    };
} // namespace

TEST(LogicIncOutputTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableIncOutput testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicIncOutputTestsGroup, DoAction_change_state_to_passive__due_incoming_switch_to_passive) {
    TestableIncOutput testable;
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

TEST(LogicIncOutputTestsGroup, DoAction_change_state_to_active_and_increment_value) {
    TestableIncOutput testable;
    testable.SetIoAdr(MapIO::V1);

    Controller::SampleIOValues();
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    Controller::CommitChanges();
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    Controller::V1.ReadValue();
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(1, Controller::V1.ReadValue());
}

TEST(LogicIncOutputTestsGroup,
     DoAction_change_state_to_active__and_second_call_does_not_increment) {
    TestableIncOutput testable;
    testable.SetIoAdr(MapIO::V1);

    Controller::V1.WriteValue(42);
    Controller::V1.CommitChanges();

    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    Controller::CommitChanges();    
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(43, Controller::V1.PeekValue());

    CHECK_FALSE(Controller::SampleIOValues());
    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    Controller::CommitChanges();    
    CHECK_EQUAL(43, Controller::V1.PeekValue());
}

TEST(LogicIncOutputTestsGroup, DoAction_change_state_to_passive) {
    Controller::V1.WriteValue(42);
    Controller::V1.CommitChanges(); 

    TestableIncOutput testable;
    testable.SetIoAdr(MapIO::V1);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    Controller::CommitChanges(); 
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicIncOutputTestsGroup, GetElementType_returns_et_IncOutput) {
    TestableIncOutput testable;
    CHECK_EQUAL(TvElementType::et_IncOutput, testable.PublicMorozov_GetElementType());
}

TEST(LogicIncOutputTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableIncOutput testable;
    testable.SetIoAdr(MapIO::O1);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_IncOutput, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::O1, *((MapIO *)&buffer[1]));
}

TEST(LogicIncOutputTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_IncOutput;
    *((MapIO *)&buffer[1]) = MapIO::O2;

    TestableIncOutput testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);
    CHECK_EQUAL(MapIO::O2, testable.GetIoAdr());
    CHECK(&Controller::O2 == testable.Output);
    CHECK(&Controller::O2 == testable.Input);
}

TEST(LogicIncOutputTestsGroup, GetElementType) {
    TestableIncOutput testable;
    CHECK_EQUAL(TvElementType::et_IncOutput, testable.GetElementType());
}

TEST(LogicIncOutputTestsGroup, TryToCast) {
    DirectOutput directOutput;
    CHECK_TRUE(IncOutput::TryToCast(&directOutput) == NULL);

    SetOutput setOutput;
    CHECK_TRUE(IncOutput::TryToCast(&setOutput) == NULL);

    ResetOutput resetOutput;
    CHECK_TRUE(IncOutput::TryToCast(&resetOutput) == NULL);

    IncOutput incOutput;
    CHECK_TRUE(IncOutput::TryToCast(&incOutput) == &incOutput);

    DecOutput decOutput;
    CHECK_TRUE(IncOutput::TryToCast(&decOutput) == NULL);
}