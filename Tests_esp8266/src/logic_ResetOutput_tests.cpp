#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/IncomeRail.h"
#include "main/LogicProgram/Outputs/ResetOutput.h"
#include "main/LogicProgram/Inputs/InputNC.h"

TEST_GROUP(LogicResetOutputTestsGroup){ //
                                      TEST_SETUP(){}

                                      TEST_TEARDOWN(){}
};

namespace {
    class TestableResetOutput : public ResetOutput {
      public:
        TestableResetOutput(const MapIO io_adr, InputBase *incoming_item)
            : ResetOutput(io_adr, incoming_item) {
        }
        virtual ~TestableResetOutput() {
        }
        InputBase *PublicMorozov_incoming_item() {
            return incoming_item;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };

    class TestableInputNC : public InputNC {
      public:
        TestableInputNC(const MapIO io_adr, InputBase *incoming_item)
            : InputNC(io_adr, incoming_item) {
        }
        virtual ~TestableInputNC() {
        }

        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };
} // namespace

TEST(LogicResetOutputTestsGroup, DoAction_skip_when_incoming_passive) {
    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 0, LogicItemState::lisPassive);
    TestableResetOutput testable(MapIO::V1, &incomeRail);

    CHECK_FALSE(testable.DoAction(false));
    CHECK_EQUAL(LogicItemState::lisPassive, testable.GetState());
}

TEST(LogicResetOutputTestsGroup,
     DoAction_change_state_to_active__and_second_call_does_nothing) {
    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 0, LogicItemState::lisActive);
    TestableResetOutput testable(MapIO::V1, &incomeRail);

    controller.SetV1RelativeValue(StatefulElement::MaxValue);

    CHECK_TRUE(testable.DoAction(false));
    CHECK_EQUAL(LogicItemState::lisActive, testable.GetState());
    CHECK_EQUAL(StatefulElement::MinValue, controller.GetV1RelativeValue());
    
    CHECK_FALSE(testable.DoAction(false));
    CHECK_EQUAL(StatefulElement::MinValue, controller.GetV1RelativeValue());
}

TEST(LogicResetOutputTestsGroup, DoAction_change_state_to_passive) {
    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 0, LogicItemState::lisActive);
    TestableInputNC prev_element(MapIO::DI, &incomeRail);

    controller.SetV1RelativeValue(StatefulElement::MaxValue);

    TestableResetOutput testable(MapIO::V1, &prev_element);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;
    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisPassive;

    CHECK_TRUE(testable.DoAction(false));
    CHECK_EQUAL(LogicItemState::lisPassive, testable.GetState());
    CHECK_EQUAL(StatefulElement::MaxValue, controller.GetV1RelativeValue());
}
