#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/IncomeRail.h"
#include "main/LogicProgram/Outputs/SetOutput.h"
#include "main/LogicProgram/Inputs/InputNC.h"

TEST_GROUP(LogicSetOutputTestsGroup){ //
                                      TEST_SETUP(){}

                                      TEST_TEARDOWN(){}
};

namespace {
    class TestableSetOutput : public SetOutput {
      public:
        TestableSetOutput(const MapIO io_adr, InputBase *incoming_item)
            : SetOutput(io_adr, incoming_item) {
        }
        virtual ~TestableSetOutput() {
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

TEST(LogicSetOutputTestsGroup, DoAction_skip_when_incoming_passive) {
    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 0, LogicItemState::lisPassive);
    TestableSetOutput testable(MapIO::V1, &incomeRail);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicSetOutputTestsGroup,
     DoAction_change_state_to_active__and_second_call_does_nothing) {
    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 0, LogicItemState::lisActive);
    TestableSetOutput testable(MapIO::V1, &incomeRail);

    controller.SetV1RelativeValue(LogicElement::MinValue);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicElement::MaxValue, controller.GetV1RelativeValue());
    
    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicElement::MaxValue, controller.GetV1RelativeValue());
}

TEST(LogicSetOutputTestsGroup, DoAction_change_state_to_passive) {
    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 0, LogicItemState::lisActive);
    TestableInputNC prev_element(MapIO::DI, &incomeRail);

    controller.SetV1RelativeValue(LogicElement::MinValue);

    TestableSetOutput testable(MapIO::V1, &prev_element);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;
    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisPassive;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicElement::MinValue, controller.GetV1RelativeValue());
}
