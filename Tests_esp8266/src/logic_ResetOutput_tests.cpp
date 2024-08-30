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
#include "main/LogicProgram/Outputs/ResetOutput.h"

TEST_GROUP(LogicResetOutputTestsGroup){ //
                                        TEST_SETUP(){}

                                        TEST_TEARDOWN(){}
};

namespace {
    class TestableResetOutput : public ResetOutput {
      public:
        TestableResetOutput(const MapIO io_adr)
            : ResetOutput(io_adr) {
        }
        virtual ~TestableResetOutput() {
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };
} // namespace

TEST(LogicResetOutputTestsGroup, DoAction_skip_when_incoming_passive) {
    
    IncomeRail incomeRail(0, LogicItemState::lisPassive);
    TestableResetOutput testable(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicResetOutputTestsGroup, DoAction_change_state_to_active__and_second_call_does_nothing) {
    
    IncomeRail incomeRail(0, LogicItemState::lisActive);
    TestableResetOutput testable(MapIO::V1);

    Controller::SetV1RelativeValue(LogicElement::MaxValue);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicElement::MinValue, Controller::GetV1RelativeValue());

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicElement::MinValue, Controller::GetV1RelativeValue());
}

TEST(LogicResetOutputTestsGroup, DoAction_change_state_to_passive) {
    
    IncomeRail incomeRail(0, LogicItemState::lisActive);

    Controller::SetV1RelativeValue(LogicElement::MaxValue);

    TestableResetOutput testable(MapIO::V1);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::GetV1RelativeValue());
}
