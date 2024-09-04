#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/LogicProgram.h"
#include "main/LogicProgram/Network.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicNetworkTestsGroup){ //
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

    class TestableNetwork : public Network {
      public:
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

    class TestableInputNC : public InputNC, public MonitorLogicElement {
      public:
        TestableInputNC() : InputNC() {
        }

        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            return MonitorLogicElement::DoAction();
        }

        bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            (void)start_point;
            return MonitorLogicElement::Render();
        }
    };

    class TestableComparatorEq : public ComparatorEq, public MonitorLogicElement {
      public:
        TestableComparatorEq() : ComparatorEq() {
        }
        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            return MonitorLogicElement::DoAction();
        }

        bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            (void)start_point;
            return MonitorLogicElement::Render();
        }
    };

    class TestableTimerMSecs : public TimerMSecs, public MonitorLogicElement {
      public:
        explicit TestableTimerMSecs() : TimerMSecs() {
        }

        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            return MonitorLogicElement::DoAction();
        }

        bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            (void)start_point;
            return MonitorLogicElement::Render();
        }
    };

    class TestableDirectOutput : public DirectOutput, public MonitorLogicElement {
      public:
        explicit TestableDirectOutput() : DirectOutput() {
        }
        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            return MonitorLogicElement::DoAction();
        }

        bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            (void)start_point;
            return MonitorLogicElement::Render();
        }
    };
} // namespace

TEST(LogicNetworkTestsGroup, append_elements) {
    TestableNetwork testable;
    testable.ChangeState(LogicItemState::lisActive);

    testable.Append(new TestableInputNC);
    testable.Append(new TestableComparatorEq());
    testable.Append(new TestableTimerMSecs());
    testable.Append(new TestableDirectOutput);

    CHECK_EQUAL(4, testable.size());
}

TEST(LogicNetworkTestsGroup, DoAction_handle_all_logic_elements_in_chain) {
    TestableNetwork testable;
    testable.ChangeState(LogicItemState::lisActive);

    testable.Append(new TestableInputNC);
    testable.Append(new TestableComparatorEq());
    testable.Append(new TestableTimerMSecs());
    testable.Append(new TestableDirectOutput);

    testable.DoAction();

    CHECK_TRUE(static_cast<TestableInputNC *>(testable[0])->DoAction_called);
    CHECK_TRUE(static_cast<TestableComparatorEq *>(testable[1])->DoAction_called);
    CHECK_TRUE(static_cast<TestableTimerMSecs *>(testable[2])->DoAction_called);
    CHECK_TRUE(static_cast<TestableDirectOutput *>(testable[3])->DoAction_called);
}

TEST(LogicNetworkTestsGroup, DoAction_return_changes_from_any_handler_in_chain) {
    TestableNetwork testable;
    testable.ChangeState(LogicItemState::lisActive);

    testable.Append(new TestableInputNC);
    testable.Append(new TestableComparatorEq());
    testable.Append(new TestableTimerMSecs());
    testable.Append(new TestableDirectOutput);

    bool res = testable.DoAction();
    CHECK_FALSE(res);

    static_cast<TestableTimerMSecs *>(testable[2])->DoAction_result = true;
    res = testable.DoAction();
    CHECK_TRUE(res);

    static_cast<TestableTimerMSecs *>(testable[2])->DoAction_result = false;
    static_cast<TestableDirectOutput *>(testable[3])->DoAction_result = true;
    res = testable.DoAction();
    CHECK_TRUE(res);
}

TEST(LogicNetworkTestsGroup, Render_when_active__also_render_all_elements_in_chain) {
    TestableNetwork testable;
    testable.ChangeState(LogicItemState::lisActive);

    testable.Append(new TestableInputNC);
    testable.Append(new TestableComparatorEq());
    testable.Append(new TestableTimerMSecs());
    testable.Append(new TestableDirectOutput);

    CHECK_TRUE(testable.Render(frame_buffer));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);

    CHECK_TRUE(static_cast<TestableInputNC *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableComparatorEq *>(testable[1])->Render_called);
    CHECK_TRUE(static_cast<TestableTimerMSecs *>(testable[2])->Render_called);
    CHECK_TRUE(static_cast<TestableDirectOutput *>(testable[3])->Render_called);
}

TEST(LogicNetworkTestsGroup, Render_when_passive__also_render_all_elements_in_chain) {
    TestableNetwork testable;
    testable.ChangeState(LogicItemState::lisActive);

    testable.Append(new TestableInputNC);
    testable.Append(new TestableComparatorEq());
    testable.Append(new TestableTimerMSecs());
    testable.Append(new TestableDirectOutput);

    CHECK_TRUE(testable.Render(frame_buffer));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);

    CHECK_TRUE(static_cast<TestableInputNC *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableComparatorEq *>(testable[1])->Render_called);
    CHECK_TRUE(static_cast<TestableTimerMSecs *>(testable[2])->Render_called);
    CHECK_TRUE(static_cast<TestableDirectOutput *>(testable[3])->Render_called);
}

TEST(LogicNetworkTestsGroup, render_error_in_any_element_in_chain_is_break_process) {
    TestableNetwork testable;
    testable.ChangeState(LogicItemState::lisActive);

    testable.Append(new TestableInputNC);
    testable.Append(new TestableComparatorEq());
    testable.Append(new TestableTimerMSecs());
    testable.Append(new TestableDirectOutput);
    static_cast<TestableComparatorEq *>(testable[1])->Render_result = false;

    CHECK_FALSE(testable.Render(frame_buffer));

    CHECK_TRUE(static_cast<TestableInputNC *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableComparatorEq *>(testable[1])->Render_called);
    CHECK_FALSE(static_cast<TestableTimerMSecs *>(testable[2])->Render_called);
    CHECK_FALSE(static_cast<TestableDirectOutput *>(testable[3])->Render_called);
}

TEST(LogicNetworkTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableNetwork testable;
    testable.ChangeState(LogicItemState::lisActive);
    auto input = new TestableInputNC;
    input->SetIoAdr(MapIO::DI);
    testable.Append(input);
    auto comparator = new TestableComparatorEq();
    comparator->SetReference(5);
    comparator->SetIoAdr(MapIO::AI);
    testable.Append(comparator);
    auto timer = new TestableTimerMSecs;
    timer->SetTime(12345);
    testable.Append(timer);
    auto output = new TestableDirectOutput;
    output->SetIoAdr(MapIO::O1);
    testable.Append(output);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(19, writed);

    CHECK_EQUAL(LogicItemState::lisActive, *((LogicItemState *)&buffer[0]));
    CHECK_EQUAL(4, *((uint16_t *)&buffer[1]));

    CHECK_EQUAL(TvElementType::et_InputNC, *((TvElementType *)&buffer[3]));
    CHECK_EQUAL(MapIO::DI, *((MapIO *)&buffer[4]));

    CHECK_EQUAL(TvElementType::et_ComparatorEq, *((TvElementType *)&buffer[5]));
    CHECK_EQUAL(5, *((uint8_t *)&buffer[6]));
    CHECK_EQUAL(MapIO::AI, *((MapIO *)&buffer[7]));

    CHECK_EQUAL(TvElementType::et_TimerMSecs, *((TvElementType *)&buffer[8]));
    CHECK_EQUAL(12345000, *((uint64_t *)&buffer[9]));

    CHECK_EQUAL(TvElementType::et_DirectOutput, *((TvElementType *)&buffer[17]));
    CHECK_EQUAL(MapIO::O1, *((MapIO *)&buffer[18]));
}

TEST(LogicNetworkTestsGroup, Serialize_just_for_obtain_size) {
    TestableNetwork testable;
    testable.ChangeState(LogicItemState::lisActive);
    testable.Append(new TestableInputNC);
    testable.Append(new TestableComparatorEq());
    testable.Append(new TestableTimerMSecs());
    testable.Append(new TestableDirectOutput);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(19, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(19, writed);
}

TEST(LogicNetworkTestsGroup, Serialize_when_elemens_count_exceed_max__return_zero) {
    uint8_t buffer[256] = {};
    TestableNetwork testable;
    testable.ChangeState(LogicItemState::lisActive);

    for (size_t i = 0; i < 6; i++) {
        testable.Append(new TestableInputNC);
    }

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicNetworkTestsGroup, Serialize_when_elemens_count_less_than_min__return_zero) {
    uint8_t buffer[256] = {};
    TestableNetwork testable;
    testable.ChangeState(LogicItemState::lisActive);

    testable.Append(new TestableInputNC);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicNetworkTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[256] = {};
    TestableNetwork testable;
    testable.ChangeState(LogicItemState::lisActive);
    testable.Append(new TestableInputNC);
    testable.Append(new TestableComparatorEq());
    testable.Append(new TestableTimerMSecs());
    testable.Append(new TestableDirectOutput);

    size_t writed = testable.Serialize(buffer, 20);
    CHECK_EQUAL(19, writed);

    for (size_t i = 0; i < 19; i++) {
        writed = testable.Serialize(buffer, i);
        CHECK_EQUAL(0, writed);
    }
}

TEST(LogicNetworkTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((LogicItemState *)&buffer[0]) = LogicItemState::lisActive;
    *((uint16_t *)&buffer[1]) = 4;

    *((TvElementType *)&buffer[3]) = TvElementType::et_InputNC;
    *((MapIO *)&buffer[4]) = MapIO::DI;

    *((TvElementType *)&buffer[5]) = TvElementType::et_ComparatorEq;
    *((uint8_t *)&buffer[6]) = 5;
    *((MapIO *)&buffer[7]) = MapIO::AI;

    *((TvElementType *)&buffer[8]) = TvElementType::et_TimerMSecs;
    *((uint64_t *)&buffer[9]) = 12345000;

    *((TvElementType *)&buffer[17]) = TvElementType::et_DirectOutput;
    *((MapIO *)&buffer[18]) = MapIO::O1;

    TestableNetwork testable;
    testable.ChangeState(LogicItemState::lisActive);

    size_t readed = testable.Deserialize(&buffer[0], sizeof(buffer) - 1);
    CHECK_EQUAL(19, readed);
    CHECK_EQUAL(4, testable.size());
    CHECK_EQUAL(TvElementType::et_InputNC, testable[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable[1]->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable[2]->GetElementType());
    CHECK_EQUAL(TvElementType::et_DirectOutput, testable[3]->GetElementType());
}