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

TEST_GROUP(LogicDecOutputTestsGroup){ //
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
    class TestableDecOutput : public DecOutput {
      public:
        TestableDecOutput() : DecOutput() {
        }
        virtual ~TestableDecOutput() {
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        TvElementType PublicMorozov_GetElementType() {
            return GetElementType();
        }
    };
} // namespace

TEST(LogicDecOutputTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableDecOutput testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicDecOutputTestsGroup, DoAction_change_state_to_passive__due_incoming_switch_to_passive) {
    TestableDecOutput testable;
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

TEST(LogicDecOutputTestsGroup,
     DoAction_change_state_to_active__and_second_call_does_not_decrement) {
    TestableDecOutput testable;
    testable.SetIoAdr(MapIO::V1);
    Controller::V1.WriteValue(42);

    Controller::FetchIOValues();
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    Controller::CommitChanges();    
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    CHECK_EQUAL(41, Controller::V1.PeekValue());

    Controller::FetchIOValues();
    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    Controller::CommitChanges();    
    CHECK_EQUAL(41, Controller::V1.PeekValue());
}

TEST(LogicDecOutputTestsGroup, DoAction_change_state_to_passive) {
    Controller::V1.WriteValue(42);

    TestableDecOutput testable;
    testable.SetIoAdr(MapIO::V1);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    Controller::FetchIOValues();
    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    Controller::CommitChanges(); 
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicDecOutputTestsGroup, GetElementType_returns_et_DecOutput) {
    TestableDecOutput testable;
    CHECK_EQUAL(TvElementType::et_DecOutput, testable.PublicMorozov_GetElementType());
}

TEST(LogicDecOutputTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableDecOutput testable;
    testable.SetIoAdr(MapIO::O1);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_DecOutput, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::O1, *((MapIO *)&buffer[1]));
}

TEST(LogicDecOutputTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_DecOutput;
    *((MapIO *)&buffer[1]) = MapIO::O2;

    TestableDecOutput testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);
    CHECK_EQUAL(MapIO::O2, testable.GetIoAdr());
    CHECK(&Controller::O2 == testable.Output);
    CHECK(&Controller::O2 == testable.Input);
}

TEST(LogicDecOutputTestsGroup, GetElementType) {
    TestableDecOutput testable;
    CHECK_EQUAL(TvElementType::et_DecOutput, testable.GetElementType());
}

TEST(LogicDecOutputTestsGroup, TryToCast) {
    DirectOutput directOutput;
    CHECK_TRUE(DecOutput::TryToCast(&directOutput) == NULL);

    SetOutput setOutput;
    CHECK_TRUE(DecOutput::TryToCast(&setOutput) == NULL);

    ResetOutput resetOutput;
    CHECK_TRUE(DecOutput::TryToCast(&resetOutput) == NULL);

    IncOutput incOutput;
    CHECK_TRUE(DecOutput::TryToCast(&incOutput) == NULL);

    DecOutput decOutput;
    CHECK_TRUE(DecOutput::TryToCast(&decOutput) == &decOutput);
}