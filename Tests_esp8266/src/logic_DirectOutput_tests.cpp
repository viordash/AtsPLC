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
#include "main/LogicProgram/Outputs/DirectOutput.h"

TEST_GROUP(LogicDirectOutputTestsGroup){ //
                                         TEST_SETUP(){}

                                         TEST_TEARDOWN(){}
};

namespace {
    class TestableDirectOutput : public DirectOutput {
      public:
        TestableDirectOutput(const MapIO io_adr, InputBase *incoming_item)
            : DirectOutput(io_adr, incoming_item) {
        }
        virtual ~TestableDirectOutput() {
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

TEST(LogicDirectOutputTestsGroup, DoAction_skip_when_incoming_passive) {
    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 0, LogicItemState::lisPassive);
    TestableDirectOutput testable(MapIO::V1, &incomeRail);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicDirectOutputTestsGroup, DoAction_change_state_to_active) {
    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 0, LogicItemState::lisActive);
    TestableDirectOutput testable(MapIO::V1, &incomeRail);

    controller.SetV1RelativeValue(LogicElement::MinValue);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicElement::MaxValue, controller.GetV1RelativeValue());
}

TEST(LogicDirectOutputTestsGroup, DoAction_change_state_to_passive) {
    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 0, LogicItemState::lisActive);
    TestableInputNC prev_element(MapIO::DI, &incomeRail);

    controller.SetV1RelativeValue(LogicElement::MaxValue);

    TestableDirectOutput testable(MapIO::V1, &prev_element);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;
    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisPassive;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicElement::MinValue, controller.GetV1RelativeValue());
}
