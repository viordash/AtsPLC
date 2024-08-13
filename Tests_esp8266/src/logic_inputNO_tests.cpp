#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/IncomeRail.h"
#include "main/LogicProgram/Inputs/InputNO.cpp"
#include "main/LogicProgram/Inputs/InputNO.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicInputNOTestsGroup){ //
                                    TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
}

TEST_TEARDOWN() {
}
}
;

namespace {
    class TestableInputNO : public InputNO {
      public:
        TestableInputNO(const MapIO io_adr, InputBase *incoming_item)
            : InputNO(io_adr, incoming_item) {
        }
        virtual ~TestableInputNO() {
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

TEST(LogicInputNOTestsGroup, GetLabel_DI) {
    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableInputNO testable(MapIO::DI, &incomeRail0);
    STRCMP_EQUAL("DI", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, GetLabel_AI) {
    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableInputNO testable(MapIO::AI, &incomeRail0);
    STRCMP_EQUAL("AI", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, GetLabel_V1) {
    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableInputNO testable(MapIO::V1, &incomeRail0);
    STRCMP_EQUAL("V1", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, Passive_is_init_state) {
    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableInputNO testable_0(MapIO::V1, &incomeRail0);
    TestableInputNO testable_1(MapIO::V2, &testable_0);
    CHECK_EQUAL(LogicItemState::lisPassive, testable_0.GetState());
    CHECK_EQUAL(LogicItemState::lisPassive, testable_1.GetState());
}

TEST(LogicInputNOTestsGroup, chain_of_items) {
    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableInputNO testable_0(MapIO::V1, &incomeRail0);
    TestableInputNO testable_1(MapIO::V2, &testable_0);
    TestableInputNO testable_2(MapIO::V3, &testable_1);
    CHECK_EQUAL(&incomeRail0, testable_0.PublicMorozov_incoming_item());
    CHECK_EQUAL(&testable_0, testable_1.PublicMorozov_incoming_item());
    CHECK_EQUAL(&testable_1, testable_2.PublicMorozov_incoming_item());
}

TEST(LogicInputNOTestsGroup, Render_happened_on_startup) {

    Controller controller;
    IncomeRail incomeRail(controller, 0);
    TestableInputNO testable(MapIO::V1, &incomeRail);

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

TEST(LogicInputNOTestsGroup, re_render_on_demand) {

    Controller controller;
    IncomeRail incomeRail(controller, 0);
    TestableInputNO testable(MapIO::V1, &incomeRail);

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

TEST(LogicInputNOTestsGroup, DoAction_skip_when_incoming_passive) {
    mock("0").expectNoCall("gpio_get_level");

    Controller controller;
    IncomeRail incomeRail(controller, 0);
    TestableInputNO prev_element(MapIO::V1, &incomeRail);
    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisPassive;

    TestableInputNO testable(MapIO::DI, &prev_element);
    testable.Render(frame_buffer);

    CHECK_FALSE(testable.DoAction());
    CHECK_EQUAL(LogicItemState::lisPassive, testable.GetState());
    CHECK_FALSE_TEXT(*(testable.PublicMorozov_Get_require_render()),
                     "no require_render because state hasn't changed");
}

TEST(LogicInputNOTestsGroup, DoAction_change_state_to_active) {
    mock("0").expectOneCall("gpio_get_level").andReturnValue(0);

    Controller controller;
    IncomeRail incomeRail(controller, 0);
    TestableInputNO prev_element(MapIO::V1, &incomeRail);
    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    TestableInputNO testable(MapIO::DI, &prev_element);

    CHECK_TRUE(testable.DoAction());
    CHECK_EQUAL(LogicItemState::lisActive, testable.GetState());
    CHECK_TRUE_TEXT(*(testable.PublicMorozov_Get_require_render()),
                    "require_render because state has changed");
}

TEST(LogicInputNOTestsGroup, DoAction_change_state_to_passive) {
    mock("0").expectOneCall("gpio_get_level").andReturnValue(1);

    Controller controller;
    IncomeRail incomeRail(controller, 0);
    TestableInputNO prev_element(MapIO::V1, &incomeRail);
    *(prev_element.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    TestableInputNO testable(MapIO::DI, &prev_element);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction());
    CHECK_EQUAL(LogicItemState::lisPassive, testable.GetState());
    CHECK_TRUE_TEXT(*(testable.PublicMorozov_Get_require_render()),
                    "require_render because state has changed");
}
