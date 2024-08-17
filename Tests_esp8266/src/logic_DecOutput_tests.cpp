#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/IncomeRail.h"
#include "main/LogicProgram/Outputs/DecOutput.cpp"
#include "main/LogicProgram/Outputs/DecOutput.h"

TEST_GROUP(LogicDecOutputTestsGroup){ //
                                      TEST_SETUP(){}

                                      TEST_TEARDOWN(){}
};

namespace {
    class TestableDecOutput : public DecOutput {
      public:
        TestableDecOutput(const MapIO io_adr, InputBase *incoming_item)
            : DecOutput(io_adr, incoming_item) {
        }
        virtual ~TestableDecOutput() {
        }
        InputBase *PublicMorozov_incoming_item() {
            return incoming_item;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };
} // namespace

// TEST(LogicDecOutputTestsGroup, DoAction_skip_when_incoming_passive) {
//     mock("0").expectNoCall("gpio_get_level");

//     Controller controller(NULL);
//     IncomeRail incomeRail(&controller, 0);
//     TestableDecOutput testable(MapIO::V1, &incomeRail);

//     CHECK_FALSE(testable.DoAction());
//     CHECK_EQUAL(LogicItemState::lisPassive, testable.GetState());
// }

// TEST(LogicDecOutputTestsGroup, DoAction_change_state_to_active) {
//     mock("0").expectOneCall("gpio_get_level").andReturnValue(1);

//     Controller controller(NULL);
//     IncomeRail incomeRail(&controller, 0);
//     TestableDecOutput prev_element(MapIO::V1, &incomeRail);
//     *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisActive;

//     TestableDecOutput testable(MapIO::V2, &prev_element);

//     CHECK_TRUE(testable.DoAction());
//     CHECK_EQUAL(LogicItemState::lisActive, testable.GetState());
// }

// TEST(LogicDecOutputTestsGroup, DoAction_change_state_to_passive) {
//     mock("0").expectOneCall("gpio_get_level").andReturnValue(0);

//     Controller controller(NULL);
//     IncomeRail incomeRail(&controller, 0);
//     TestableDecOutput prev_element(MapIO::V1, &incomeRail);
//     *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisActive;

//     TestableDecOutput testable(MapIO::DI, &prev_element);
//     *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

//     CHECK_TRUE(testable.DoAction());
//     CHECK_EQUAL(LogicItemState::lisPassive, testable.GetState());
// }
