#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/IncomeRail.h"
#include "main/LogicProgram/Inputs/InputNC.cpp"
#include "main/LogicProgram/Inputs/InputNC.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicInputNCTestsGroup){ //
                                    TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
}

TEST_TEARDOWN() {
}
}
;

namespace {
    class TestableInputNC : public InputNC {
      public:
        TestableInputNC(const MapIO io_adr, InputBase *incoming_item)
            : InputNC(io_adr, incoming_item) {
        }
        virtual ~TestableInputNC() {
        }

        const char *GetLabel() {
            return label;
        }
        InputBase *PublicMorozov_incoming_item() {
            return incoming_item;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };
} // namespace

TEST(LogicInputNCTestsGroup, DoAction_skip_when_incoming_passive) {
    mock("0").expectNoCall("gpio_get_level");

    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 0, LogicItemState::lisPassive);

    TestableInputNC testable(MapIO::DI, &incomeRail);

    CHECK_FALSE(testable.DoAction(false));
    CHECK_EQUAL(LogicItemState::lisPassive, testable.GetState());
}

TEST(LogicInputNCTestsGroup, DoAction_change_state_to_active) {
    mock("0").expectOneCall("gpio_get_level").andReturnValue(1);

    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 0, LogicItemState::lisActive);

    TestableInputNC testable(MapIO::DI, &incomeRail);

    CHECK_TRUE(testable.DoAction(false));
    CHECK_EQUAL(LogicItemState::lisActive, testable.GetState());
}

TEST(LogicInputNCTestsGroup, DoAction_change_state_to_passive) {
    mock("0").expectOneCall("gpio_get_level").andReturnValue(0);

    Controller controller(NULL);
    IncomeRail incomeRail(&controller, 0, LogicItemState::lisActive);

    TestableInputNC testable(MapIO::DI, &incomeRail);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(false));
    CHECK_EQUAL(LogicItemState::lisPassive, testable.GetState());
}
