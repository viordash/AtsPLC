#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/IncomeRail.h"
#include "main/LogicProgram/LogicProgram.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicIncomeRailTestsGroup){ //
                                       TEST_SETUP(){ mock().disable();
memset(frame_buffer, 0, sizeof(frame_buffer));
}

TEST_TEARDOWN() {
    mock().enable();
}
}
;

namespace {
    static const Bitmap bitmap = { //
        { 16,                      // width
          16 },                    // height
        { 0xFF, 0x00, 0x00, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x0A, 0x0A, 0x0A,
          0x0A, 0x00, 0x00, 0xFF, 0x80, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01 }
    };

    class TestableIncomeRail : public IncomeRail {
      public:
        TestableIncomeRail(uint8_t network_number, LogicItemState state)
            : IncomeRail(network_number, state) {
        }
    };

    class TestableCommonInput : public CommonInput {
      public:
        TestableCommonInput(const MapIO io_adr) : CommonInput(io_adr) {
        }
        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }

        bool DoAction_called = false;
        bool DoAction_result = false;
        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            DoAction_called = true;
            return DoAction_result;
        }

        bool Render_called = false;
        bool Render_result = true;
        bool
        Render(uint8_t *fb, LogicItemState prev_elem_state, const Point &start_point) override {
            (void)fb;
            (void)prev_elem_state;
            (void)start_point;
            Render_called = true;
            return Render_result;
        }
    };

    class TestableCommonComparator : public CommonComparator {
      public:
        TestableCommonComparator(uint16_t reference, const MapIO io_adr)
            : CommonComparator(reference, io_adr) {
        }
        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }

        bool DoAction_called = false;
        bool DoAction_result = false;
        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            DoAction_called = true;
            return DoAction_result;
        }
        bool CompareFunction() override {
            return true;
        }

        bool Render_called = false;
        bool Render_result = true;
        bool
        Render(uint8_t *fb, LogicItemState prev_elem_state, const Point &start_point) override {
            (void)fb;
            (void)prev_elem_state;
            (void)start_point;
            Render_called = true;
            return Render_result;
        }
    };

    class TestableCommonTimer : public CommonTimer {
      public:
        explicit TestableCommonTimer() : CommonTimer() {
        }
        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }

        bool DoAction_called = false;
        bool DoAction_result = false;
        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            DoAction_called = true;
            return DoAction_result;
        }

        bool Render_called = false;
        bool Render_result = true;
        bool
        Render(uint8_t *fb, LogicItemState prev_elem_state, const Point &start_point) override {
            (void)fb;
            (void)prev_elem_state;
            (void)start_point;
            Render_called = true;
            return Render_result;
        }
    };

    class TestableCommonOutput : public CommonOutput {
      public:
        TestableCommonOutput(const MapIO io_adr) : CommonOutput(io_adr) {
        }
        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }

        bool DoAction_called = false;
        bool DoAction_result = false;
        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            DoAction_called = true;
            return DoAction_result;
        }

        bool Render_called = false;
        bool Render_result = true;
        bool
        Render(uint8_t *fb, LogicItemState prev_elem_state, const Point &start_point) override {
            (void)fb;
            (void)prev_elem_state;
            (void)start_point;
            Render_called = true;
            return Render_result;
        }
    };

} // namespace

// IGNORE_TEST(LogicIncomeRailTestsGroup, Chain_of_logic_elements) {

//
//     TestableIncomeRail testable(0, LogicItemState::lisActive);

//     TestableCommonInput input1(MapIO::DI);
//     TestableCommonComparator comparator1(5, MapIO::AI);
//     TestableCommonTimer timerSecs1;
//     TestableCommonOutput directOutput0(MapIO::O1);
//     OutcomeRail outcomeRail0(0);

//     LogicElement *nextElement = testable.PublicMorozov_GetNext();
//     CHECK_EQUAL(&input1, nextElement);

//     nextElement = input1.PublicMorozov_GetNext();
//     CHECK_EQUAL(&comparator1, nextElement);

//     nextElement = comparator1.PublicMorozov_GetNext();
//     CHECK_EQUAL(&timerSecs1, nextElement);

//     nextElement = timerSecs1.PublicMorozov_GetNext();
//     CHECK_EQUAL(&directOutput0, nextElement);

//     nextElement = directOutput0.PublicMorozov_GetNext();
//     CHECK(nextElement == NULL);
// }

IGNORE_TEST(LogicIncomeRailTestsGroup, DoAction_handle_all_logic_elements_in_chain) {

    TestableIncomeRail testable(0, LogicItemState::lisActive);

    TestableCommonInput input1(MapIO::DI);
    TestableCommonComparator comparator1(5, MapIO::AI);
    TestableCommonTimer timerSecs1;
    TestableCommonOutput directOutput0(MapIO::O1);
    OutcomeRail outcomeRail0(0);

    testable.DoAction();

    CHECK_TRUE(input1.DoAction_called);
    CHECK_TRUE(comparator1.DoAction_called);
    CHECK_TRUE(timerSecs1.DoAction_called);
    CHECK_TRUE(directOutput0.DoAction_called);
}

IGNORE_TEST(LogicIncomeRailTestsGroup, DoAction_return_changes_from_any_handler_in_chain) {

    TestableIncomeRail testable(0, LogicItemState::lisActive);

    TestableCommonInput input1(MapIO::DI);
    TestableCommonComparator comparator1(5, MapIO::AI);
    TestableCommonTimer timerSecs1;
    TestableCommonOutput directOutput0(MapIO::O1);
    OutcomeRail outcomeRail0(0);

    bool res = testable.DoAction();
    CHECK_FALSE(res);

    timerSecs1.DoAction_result = true;
    res = testable.DoAction();
    CHECK_TRUE(res);

    timerSecs1.DoAction_result = false;
    directOutput0.DoAction_result = true;
    res = testable.DoAction();
    CHECK_TRUE(res);
}

IGNORE_TEST(LogicIncomeRailTestsGroup, Render_when_active__also_render_all_elements_in_chain) {

    TestableIncomeRail testable(0, LogicItemState::lisActive);

    TestableCommonInput input1(MapIO::DI);
    TestableCommonComparator comparator1(5, MapIO::AI);
    TestableCommonTimer timerSecs1;
    TestableCommonOutput directOutput0(MapIO::O1);
    OutcomeRail outcomeRail0(0);

    CHECK_TRUE(testable.Render(frame_buffer));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);

    CHECK_TRUE(input1.Render_called);
    CHECK_TRUE(comparator1.Render_called);
    CHECK_TRUE(timerSecs1.Render_called);
    CHECK_TRUE(directOutput0.Render_called);
}

IGNORE_TEST(LogicIncomeRailTestsGroup, Render_when_passive__also_render_all_elements_in_chain) {

    TestableIncomeRail testable(0, LogicItemState::lisPassive);

    TestableCommonInput input1(MapIO::DI);
    TestableCommonComparator comparator1(5, MapIO::AI);
    TestableCommonTimer timerSecs1;
    TestableCommonOutput directOutput0(MapIO::O1);
    OutcomeRail outcomeRail0(0);

    CHECK_TRUE(testable.Render(frame_buffer));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);

    CHECK_TRUE(input1.Render_called);
    CHECK_TRUE(comparator1.Render_called);
    CHECK_TRUE(timerSecs1.Render_called);
    CHECK_TRUE(directOutput0.Render_called);
}

IGNORE_TEST(LogicIncomeRailTestsGroup, render_error_in_any_element_in_chain_is_break_process) {

    TestableIncomeRail testable(0, LogicItemState::lisActive);

    TestableCommonInput input1(MapIO::DI);
    TestableCommonComparator comparator1(5, MapIO::AI);
    comparator1.Render_result = false;
    TestableCommonTimer timerSecs1;
    TestableCommonOutput directOutput0(MapIO::O1);
    OutcomeRail outcomeRail0(0);

    CHECK_FALSE(testable.Render(frame_buffer));

    CHECK_TRUE(input1.Render_called);
    CHECK_TRUE(comparator1.Render_called);
    CHECK_FALSE(timerSecs1.Render_called);
    CHECK_FALSE(directOutput0.Render_called);
}