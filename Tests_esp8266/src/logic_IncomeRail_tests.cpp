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

    class MonitorLogicElement {
      public:
        bool DoAction_called = false;
        bool DoAction_result = false;
        bool Render_called = false;
        bool Render_result = true;

        bool DoAction() {
            DoAction_called = true;
            return DoAction_result;
        }

        bool Render() {
            Render_called = true;
            return Render_result;
        }
    };

    class TestableCommonInput : public CommonInput, public MonitorLogicElement {
      public:
        TestableCommonInput(const MapIO io_adr) : CommonInput(io_adr) {
        }
        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }
        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            return MonitorLogicElement::DoAction();
        }

        bool
        Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            (void)start_point;
            return MonitorLogicElement::Render();
        }
    };

    class TestableCommonComparator : public CommonComparator, public MonitorLogicElement {
      public:
        TestableCommonComparator(uint16_t reference, const MapIO io_adr)
            : CommonComparator(reference, io_adr) {
        }
        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }

        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            return MonitorLogicElement::DoAction();
        }
        bool CompareFunction() override {
            return true;
        }

        bool
        Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            (void)start_point;
            return MonitorLogicElement::Render();
        }
    };

    class TestableCommonTimer : public CommonTimer, public MonitorLogicElement {
      public:
        explicit TestableCommonTimer() : CommonTimer() {
        }
        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }

        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            return MonitorLogicElement::DoAction();
        }

        bool
        Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            (void)start_point;
            return MonitorLogicElement::Render();
        }
    };

    class TestableCommonOutput : public CommonOutput, public MonitorLogicElement {
      public:
        explicit TestableCommonOutput(const MapIO io_adr) : CommonOutput(io_adr) {
        }
        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }

        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            return MonitorLogicElement::DoAction();
        }

        bool
        Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            (void)start_point;
            return MonitorLogicElement::Render();
        }
    };
} // namespace

TEST(LogicIncomeRailTestsGroup, append_elements) {
    TestableIncomeRail testable(0, LogicItemState::lisActive);

    testable.Append(new TestableCommonInput(MapIO::DI));
    testable.Append(new TestableCommonComparator(5, MapIO::AI));
    testable.Append(new TestableCommonTimer());
    testable.Append(new TestableCommonOutput(MapIO::O1));

    CHECK_EQUAL(4, testable.size());
}

TEST(LogicIncomeRailTestsGroup, DoAction_handle_all_logic_elements_in_chain) {
    TestableIncomeRail testable(0, LogicItemState::lisActive);

    testable.Append(new TestableCommonInput(MapIO::DI));
    testable.Append(new TestableCommonComparator(5, MapIO::AI));
    testable.Append(new TestableCommonTimer());
    testable.Append(new TestableCommonOutput(MapIO::O1));

    testable.DoAction();

    CHECK_TRUE(static_cast<TestableCommonInput *>(testable[0])->DoAction_called);
    CHECK_TRUE(static_cast<TestableCommonComparator *>(testable[1])->DoAction_called);
    CHECK_TRUE(static_cast<TestableCommonTimer *>(testable[2])->DoAction_called);
    CHECK_TRUE(static_cast<TestableCommonOutput *>(testable[3])->DoAction_called);
}

TEST(LogicIncomeRailTestsGroup, DoAction_return_changes_from_any_handler_in_chain) {

    TestableIncomeRail testable(0, LogicItemState::lisActive);

    testable.Append(new TestableCommonInput(MapIO::DI));
    testable.Append(new TestableCommonComparator(5, MapIO::AI));
    testable.Append(new TestableCommonTimer());
    testable.Append(new TestableCommonOutput(MapIO::O1));

    bool res = testable.DoAction();
    CHECK_FALSE(res);

    static_cast<TestableCommonTimer *>(testable[2])->DoAction_result = true;
    res = testable.DoAction();
    CHECK_TRUE(res);

    static_cast<TestableCommonTimer *>(testable[2])->DoAction_result = false;
    static_cast<TestableCommonOutput *>(testable[3])->DoAction_result = true;
    res = testable.DoAction();
    CHECK_TRUE(res);
}

TEST(LogicIncomeRailTestsGroup, Render_when_active__also_render_all_elements_in_chain) {

    TestableIncomeRail testable(0, LogicItemState::lisActive);

    testable.Append(new TestableCommonInput(MapIO::DI));
    testable.Append(new TestableCommonComparator(5, MapIO::AI));
    testable.Append(new TestableCommonTimer());
    testable.Append(new TestableCommonOutput(MapIO::O1));

    CHECK_TRUE(testable.Render(frame_buffer));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);

    CHECK_TRUE(static_cast<TestableCommonInput *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableCommonComparator *>(testable[1])->Render_called);
    CHECK_TRUE(static_cast<TestableCommonTimer *>(testable[2])->Render_called);
    CHECK_TRUE(static_cast<TestableCommonOutput *>(testable[3])->Render_called);
}

TEST(LogicIncomeRailTestsGroup, Render_when_passive__also_render_all_elements_in_chain) {

    TestableIncomeRail testable(0, LogicItemState::lisPassive);

    testable.Append(new TestableCommonInput(MapIO::DI));
    testable.Append(new TestableCommonComparator(5, MapIO::AI));
    testable.Append(new TestableCommonTimer());
    testable.Append(new TestableCommonOutput(MapIO::O1));

    CHECK_TRUE(testable.Render(frame_buffer));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);

    CHECK_TRUE(static_cast<TestableCommonInput *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableCommonComparator *>(testable[1])->Render_called);
    CHECK_TRUE(static_cast<TestableCommonTimer *>(testable[2])->Render_called);
    CHECK_TRUE(static_cast<TestableCommonOutput *>(testable[3])->Render_called);
}

TEST(LogicIncomeRailTestsGroup, render_error_in_any_element_in_chain_is_break_process) {

    TestableIncomeRail testable(0, LogicItemState::lisActive);

    testable.Append(new TestableCommonInput(MapIO::DI));
    testable.Append(new TestableCommonComparator(5, MapIO::AI));
    testable.Append(new TestableCommonTimer());
    testable.Append(new TestableCommonOutput(MapIO::O1));
    static_cast<TestableCommonComparator *>(testable[1])->Render_result = false;

    CHECK_FALSE(testable.Render(frame_buffer));

    CHECK_TRUE(static_cast<TestableCommonInput *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableCommonComparator *>(testable[1])->Render_called);
    CHECK_FALSE(static_cast<TestableCommonTimer *>(testable[2])->Render_called);
    CHECK_FALSE(static_cast<TestableCommonOutput *>(testable[3])->Render_called);
}