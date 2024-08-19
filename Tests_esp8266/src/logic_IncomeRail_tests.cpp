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
        TestableIncomeRail(const Controller *controller,
                           uint8_t network_number,
                           LogicItemState state)
            : IncomeRail(controller, network_number, state) {
        }
        StatefulElement *PublicMorozov_GetNext() {
            return nextElement;
        }
    };

    class TestableCommonInput : public CommonInput {
      public:
        TestableCommonInput(const MapIO io_adr, InputBase *incoming_item)
            : CommonInput(io_adr, incoming_item) {
        }
        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }
        StatefulElement *PublicMorozov_GetNext() {
            return nextElement;
        }

        bool DoAction_called = false;
        bool DoAction_result = false;
        bool DoAction(bool prev_changed) override {
            DoAction_called = true;
            return DoAction_result;
        }
    };

    class TestableCommonComparator : public CommonComparator {
      public:
        TestableCommonComparator(uint16_t reference, const MapIO io_adr, InputBase *incoming_item)
            : CommonComparator(reference, io_adr, incoming_item) {
        }
        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }
        StatefulElement *PublicMorozov_GetNext() {
            return nextElement;
        }

        bool DoAction_called = false;
        bool DoAction_result = false;
        bool DoAction(bool prev_changed) override {
            DoAction_called = true;
            return DoAction_result;
        }
        bool CompareFunction() override {
            return true;
        }
    };

    class TestableCommonTimer : public CommonTimer {
      public:
        explicit TestableCommonTimer(InputBase *incoming_item) : CommonTimer(incoming_item) {
        }
        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }
        StatefulElement *PublicMorozov_GetNext() {
            return nextElement;
        }

        bool DoAction_called = false;
        bool DoAction_result = false;
        bool DoAction(bool prev_changed) override {
            DoAction_called = true;
            return DoAction_result;
        }
    };

    class TestableCommonOutput : public CommonOutput {
      public:
        TestableCommonOutput(const MapIO io_adr, InputBase *incoming_item)
            : CommonOutput(io_adr, incoming_item) {
        }
        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }
        StatefulElement *PublicMorozov_GetNext() {
            return nextElement;
        }

        bool DoAction_called = false;
        bool DoAction_result = false;
        bool DoAction(bool prev_changed) override {
            DoAction_called = true;
            return DoAction_result;
        }
    };

} // namespace

TEST(LogicIncomeRailTestsGroup, Chain_of_logic_elements) {

    Controller controller(NULL);
    TestableIncomeRail testable(&controller, 0, LogicItemState::lisActive);

    TestableCommonInput input1(MapIO::DI, &testable);
    TestableCommonComparator comparator1(5, MapIO::AI, &input1);
    TestableCommonTimer timerSecs1(&comparator1);
    TestableCommonOutput directOutput0(MapIO::O1, &timerSecs1);
    OutcomeRail outcomeRail0(0);

    StatefulElement *nextElement = testable.PublicMorozov_GetNext();
    CHECK_EQUAL(&input1, nextElement);

    nextElement = input1.PublicMorozov_GetNext();
    CHECK_EQUAL(&comparator1, nextElement);

    nextElement = comparator1.PublicMorozov_GetNext();
    CHECK_EQUAL(&timerSecs1, nextElement);

    nextElement = timerSecs1.PublicMorozov_GetNext();
    CHECK_EQUAL(&directOutput0, nextElement);

    nextElement = directOutput0.PublicMorozov_GetNext();
    CHECK(nextElement == NULL);
}

TEST(LogicIncomeRailTestsGroup, DoAction_handle_all_logic_elements_in_chain) {
    Controller controller(NULL);
    TestableIncomeRail testable(&controller, 0, LogicItemState::lisActive);

    TestableCommonInput input1(MapIO::DI, &testable);
    TestableCommonComparator comparator1(5, MapIO::AI, &input1);
    TestableCommonTimer timerSecs1(&comparator1);
    TestableCommonOutput directOutput0(MapIO::O1, &timerSecs1);
    OutcomeRail outcomeRail0(0);

    testable.DoAction();

    CHECK_TRUE(input1.DoAction_called);
    CHECK_TRUE(comparator1.DoAction_called);
    CHECK_TRUE(timerSecs1.DoAction_called);
    CHECK_TRUE(directOutput0.DoAction_called);
}

TEST(LogicIncomeRailTestsGroup, DoAction_return_changes_from_any_handler_in_chain) {
    Controller controller(NULL);
    TestableIncomeRail testable(&controller, 0, LogicItemState::lisActive);

    TestableCommonInput input1(MapIO::DI, &testable);
    TestableCommonComparator comparator1(5, MapIO::AI, &input1);
    TestableCommonTimer timerSecs1(&comparator1);
    TestableCommonOutput directOutput0(MapIO::O1, &timerSecs1);
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

TEST(LogicIncomeRailTestsGroup, Render_when_active) {

    Controller controller(NULL);
    TestableIncomeRail testable(&controller, 0, LogicItemState::lisActive);

    CHECK_TRUE(testable.Render(frame_buffer));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
}

TEST(LogicIncomeRailTestsGroup, Render_when_passive) {

    Controller controller(NULL);
    TestableIncomeRail testable(&controller, 0, LogicItemState::lisPassive);

    CHECK_TRUE(testable.Render(frame_buffer));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
}