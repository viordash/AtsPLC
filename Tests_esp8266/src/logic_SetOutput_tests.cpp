#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/IncomeRail.h"
#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Outputs/SetOutput.h"

TEST_GROUP(LogicSetOutputTestsGroup){ //
                                      TEST_SETUP(){}

                                      TEST_TEARDOWN(){}
};

namespace {
    class TestableSetOutput : public SetOutput {
      public:
        TestableSetOutput(const MapIO io_adr) : SetOutput(io_adr) {
        }
        virtual ~TestableSetOutput() {
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };
} // namespace

TEST(LogicSetOutputTestsGroup, DoAction_skip_when_incoming_passive) {

    IncomeRail incomeRail(0, LogicItemState::lisPassive);
    TestableSetOutput testable(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicSetOutputTestsGroup, DoAction_change_state_to_active__and_second_call_does_nothing) {

    IncomeRail incomeRail(0, LogicItemState::lisActive);
    TestableSetOutput testable(MapIO::V1);

    Controller::SetV1RelativeValue(LogicElement::MinValue);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::GetV1RelativeValue());

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicElement::MaxValue, Controller::GetV1RelativeValue());
}

TEST(LogicSetOutputTestsGroup, DoAction_change_state_to_passive) {

    IncomeRail incomeRail(0, LogicItemState::lisActive);

    Controller::SetV1RelativeValue(LogicElement::MinValue);

    TestableSetOutput testable(MapIO::V1);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicElement::MinValue, Controller::GetV1RelativeValue());
}
