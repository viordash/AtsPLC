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
        bool *PublicMorozov_Get_require_render() {
            return &require_render;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };
} // namespace

TEST(LogicInputNCTestsGroup, Render_happened_on_startup) {

    Controller controller;
    IncomeRail incomeRail(controller, 0);
    TestableInputNC testable(MapIO::V1, &incomeRail);

    CHECK_TRUE(testable.Render(frame_buffer));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);

    memset(frame_buffer, 0, sizeof(frame_buffer));
    any_pixel_coloring = false;
    CHECK_TRUE(testable.Render(frame_buffer));

    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_FALSE(any_pixel_coloring);
    CHECK_FALSE_TEXT(*(testable.PublicMorozov_Get_require_render()),
                     "require_render does not auto-reset");
}

TEST(LogicInputNCTestsGroup, re_render_on_demand) {

    Controller controller;
    IncomeRail incomeRail(controller, 0);
    TestableInputNC testable(MapIO::V1, &incomeRail);

    CHECK_TRUE(testable.Render(frame_buffer));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);

    memset(frame_buffer, 0, sizeof(frame_buffer));
    any_pixel_coloring = false;
    *(testable.PublicMorozov_Get_require_render()) = true;

    CHECK_TRUE(testable.Render(frame_buffer));

    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
}

TEST(LogicInputNCTestsGroup, DoAction_skip_when_incoming_passive) {
    mock("0").expectNoCall("gpio_get_level");

    Controller controller;
    IncomeRail incomeRail(controller, 0);
    TestableInputNC prev_element(MapIO::V1, &incomeRail);
    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisPassive;

    TestableInputNC testable(MapIO::DI, &prev_element);
    testable.Render(frame_buffer);

    CHECK_TRUE(testable.DoAction());
    CHECK_EQUAL(LogicItemState::lisPassive, testable.GetState());
    CHECK_FALSE_TEXT(*(testable.PublicMorozov_Get_require_render()),
                     "no require_render because state hasn't changed");
}

TEST(LogicInputNCTestsGroup, DoAction_change_state) {
    mock("0").expectOneCall("gpio_get_level").andReturnValue(1);

    Controller controller;
    IncomeRail incomeRail(controller, 0);
    TestableInputNC prev_element(MapIO::V1, &incomeRail);
    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    TestableInputNC testable(MapIO::DI, &prev_element);

    CHECK_TRUE(testable.DoAction());
    CHECK_EQUAL(LogicItemState::lisActive, testable.GetState());
    CHECK_TRUE_TEXT(*(testable.PublicMorozov_Get_require_render()),
                     "require_render because state has changed");
}
