#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Ladder.cpp"
#include "main/LogicProgram/LadderInitial.cpp"
#include "main/LogicProgram/LogicProgram.h"
#include "main/redundant_storage.h"

#include "tests_utils.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicLadderTestsGroup){ //
                                   TEST_SETUP(){ mock().disable();
memset(frame_buffer, 0, sizeof(frame_buffer));
create_storage_0();
create_storage_1();
}

TEST_TEARDOWN() {
    mock().enable();
    remove_storage_0();
    remove_storage_1();
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

    class TestableLadder : public Ladder {
      public:
        size_t *PublicMorozov_Get_view_top_index() {
            return &view_top_index;
        }
    };

    class TestableNetwork : public Network, public MonitorLogicElement {
      public:
        TestableNetwork(LogicItemState state) : Network(state) {
        }

        bool DoAction() override {
            return MonitorLogicElement::DoAction();
        }

        bool Render(uint8_t *fb, uint8_t network_number) override {
            (void)fb;
            (void)network_number;
            return MonitorLogicElement::Render();
        }
    };

    class TestableInputNC : public InputNC, public MonitorLogicElement {
      public:
        TestableInputNC(const MapIO io_adr) : InputNC(io_adr) {
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
        MapIO PublicMorozov_Get_io_adr() {
            return io_adr;
        }
    };

    class TestableComparatorEq : public ComparatorEq, public MonitorLogicElement {
      public:
        TestableComparatorEq(uint8_t ref_percent04, const MapIO io_adr)
            : ComparatorEq(ref_percent04, io_adr) {
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
        uint8_t PublicMorozov_GetReference() {
            return ref_percent04;
        }
        MapIO PublicMorozov_Get_io_adr() {
            return io_adr;
        }
    };

    class TestableTimerMSecs : public TimerMSecs, public MonitorLogicElement {
      public:
        explicit TestableTimerMSecs(uint32_t delay_time_ms) : TimerMSecs(delay_time_ms) {
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
        uint64_t PublicMorozov_GetDelayTimeUs() {
            return delay_time_us;
        }
    };

    class TestableDirectOutput : public DirectOutput, public MonitorLogicElement {
      public:
        explicit TestableDirectOutput(const MapIO io_adr) : DirectOutput(io_adr) {
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
        MapIO PublicMorozov_Get_io_adr() {
            return io_adr;
        }
    };
} // namespace

TEST(LogicLadderTestsGroup, Store_Load) {
    Ladder ladder_store;

    auto network_store = new Network(LogicItemState::lisActive);
    ladder_store.Append(network_store);

    network_store->Append(new TestableInputNC(MapIO::DI));
    network_store->Append(new TestableComparatorEq(5, MapIO::AI));
    network_store->Append(new TestableTimerMSecs(12345));
    network_store->Append(new TestableDirectOutput(MapIO::O1));
    ladder_store.Store();

    Ladder ladder_load;
    ladder_load.Load();

    CHECK_EQUAL(1, ladder_load.size());

    auto network_load = ladder_load[0];
    CHECK_EQUAL(4, network_load->size());
    CHECK_EQUAL(TvElementType::et_InputNC, (*network_load)[0]->GetElementType());
    CHECK_EQUAL(MapIO::DI, ((TestableInputNC *)(*network_load)[0])->PublicMorozov_Get_io_adr());

    CHECK_EQUAL(TvElementType::et_ComparatorEq, (*network_load)[1]->GetElementType());
    CHECK_EQUAL(5, ((TestableComparatorEq *)(*network_load)[1])->PublicMorozov_GetReference());
    CHECK_EQUAL(MapIO::AI, ((TestableInputNC *)(*network_load)[1])->PublicMorozov_Get_io_adr());

    CHECK_EQUAL(TvElementType::et_TimerMSecs, (*network_load)[2]->GetElementType());
    CHECK_EQUAL(12345000,
                ((TestableTimerMSecs *)(*network_load)[2])->PublicMorozov_GetDelayTimeUs());

    CHECK_EQUAL(TvElementType::et_DirectOutput, (*network_load)[3]->GetElementType());
    CHECK_EQUAL(MapIO::O1,
                ((TestableDirectOutput *)(*network_load)[3])->PublicMorozov_Get_io_adr());
}

TEST(LogicLadderTestsGroup, Remove_elements_before_Load) {
    Ladder ladder_store;

    auto network0 = new Network(LogicItemState::lisActive);
    network0->Append(new TestableInputNC(MapIO::DI));
    network0->Append(new TestableDirectOutput(MapIO::O1));

    auto network1 = new Network(LogicItemState::lisActive);
    network1->Append(new TestableInputNC(MapIO::V1));
    network1->Append(new TestableInputNC(MapIO::V2));
    network1->Append(new TestableDirectOutput(MapIO::O2));

    auto network2 = new Network(LogicItemState::lisActive);
    network2->Append(new TestableInputNC(MapIO::V1));
    network2->Append(new TestableInputNC(MapIO::V2));
    network2->Append(new TestableInputNC(MapIO::V3));
    network2->Append(new TestableDirectOutput(MapIO::V4));

    ladder_store.Append(network0);
    ladder_store.Append(network1);
    ladder_store.Append(network2);
    ladder_store.Store();

    Ladder ladder_load;
    ladder_load.Append(new Network());
    ladder_load.Append(new Network());
    ladder_load.Load();

    CHECK_EQUAL(3, ladder_load.size());

    CHECK_EQUAL(2, ladder_load[0]->size());
    CHECK_EQUAL(3, ladder_load[1]->size());
    CHECK_EQUAL(4, ladder_load[2]->size());
}

TEST(LogicLadderTestsGroup, initial_load_when_empty_storage) {
    Ladder ladder_load;
    ladder_load.Load();

    CHECK_EQUAL(2, ladder_load.size());

    auto network0 = ladder_load[0];
    CHECK_EQUAL(4, network0->size());
    CHECK_EQUAL(TvElementType::et_InputNO, (*network0)[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_InputNC, (*network0)[1]->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerSecs, (*network0)[2]->GetElementType());
    CHECK_EQUAL(TvElementType::et_SetOutput, (*network0)[3]->GetElementType());

    auto network1 = ladder_load[1];
    CHECK_EQUAL(4, network1->size());
    CHECK_EQUAL(TvElementType::et_InputNO, (*network1)[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_InputNO, (*network1)[1]->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerSecs, (*network1)[2]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ResetOutput, (*network1)[3]->GetElementType());
}

TEST(LogicLadderTestsGroup, Deserialize_with_clear_storage__load_initial) {
    redundant_storage storage;
    storage.size = 0;
    storage.data = NULL;
    storage.version = LADDER_VERSION;

    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            ladder_storage_name,
                            &storage);

    Ladder ladder_load;
    ladder_load.Load();
    CHECK_EQUAL(2, ladder_load.size());

    auto network0 = ladder_load[0];
    CHECK_EQUAL(4, network0->size());
    CHECK_EQUAL(TvElementType::et_InputNO, (*network0)[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_InputNC, (*network0)[1]->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerSecs, (*network0)[2]->GetElementType());
    CHECK_EQUAL(TvElementType::et_SetOutput, (*network0)[3]->GetElementType());

    auto network1 = ladder_load[1];
    CHECK_EQUAL(4, network1->size());
    CHECK_EQUAL(TvElementType::et_InputNO, (*network1)[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_InputNO, (*network1)[1]->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerSecs, (*network1)[2]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ResetOutput, (*network1)[3]->GetElementType());
}

TEST(LogicLadderTestsGroup, append_network) {
    TestableLadder testable;

    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));

    CHECK_EQUAL(3, testable.size());
}

TEST(LogicLadderTestsGroup, DoAction_return_changes_from_any_network) {
    TestableLadder testable;

    testable.Append(new TestableNetwork(LogicItemState::lisActive));
    testable.Append(new TestableNetwork(LogicItemState::lisActive));
    testable.Append(new TestableNetwork(LogicItemState::lisActive));

    CHECK_FALSE(testable.DoAction());

    CHECK_TRUE(static_cast<TestableNetwork *>(testable[0])->DoAction_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[1])->DoAction_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[2])->DoAction_called);

    static_cast<TestableNetwork *>(testable[2])->DoAction_result = true;

    CHECK_TRUE(testable.DoAction());
}

TEST(LogicLadderTestsGroup, Render__also_render_all_networks_in_viewport) {
    TestableLadder testable;

    testable.Append(new TestableNetwork(LogicItemState::lisActive));
    testable.Append(new TestableNetwork(LogicItemState::lisActive));
    testable.Append(new TestableNetwork(LogicItemState::lisActive));
    testable.Append(new TestableNetwork(LogicItemState::lisActive));
    testable.Append(new TestableNetwork(LogicItemState::lisActive));
    testable.Append(new TestableNetwork(LogicItemState::lisActive));
    testable.AutoScroll();

    CHECK_TRUE(testable.Render(frame_buffer));
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[0])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[1])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[2])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[3])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[4])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[5])->Render_called);

    static_cast<TestableNetwork *>(testable[0])->Render_called = false;
    static_cast<TestableNetwork *>(testable[1])->Render_called = false;
    static_cast<TestableNetwork *>(testable[2])->Render_called = false;
    static_cast<TestableNetwork *>(testable[3])->Render_called = false;
    static_cast<TestableNetwork *>(testable[4])->Render_called = false;
    static_cast<TestableNetwork *>(testable[5])->Render_called = false;

    testable.ScrollUp();
    CHECK_TRUE(testable.Render(frame_buffer));
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[0])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[1])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[2])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[3])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[4])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[5])->Render_called);

    static_cast<TestableNetwork *>(testable[0])->Render_called = false;
    static_cast<TestableNetwork *>(testable[1])->Render_called = false;
    static_cast<TestableNetwork *>(testable[2])->Render_called = false;
    static_cast<TestableNetwork *>(testable[3])->Render_called = false;
    static_cast<TestableNetwork *>(testable[4])->Render_called = false;
    static_cast<TestableNetwork *>(testable[5])->Render_called = false;

    testable.ScrollUp();
    CHECK_TRUE(testable.Render(frame_buffer));
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[0])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[1])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[2])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[3])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[4])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[5])->Render_called);

    static_cast<TestableNetwork *>(testable[0])->Render_called = false;
    static_cast<TestableNetwork *>(testable[1])->Render_called = false;
    static_cast<TestableNetwork *>(testable[2])->Render_called = false;
    static_cast<TestableNetwork *>(testable[3])->Render_called = false;
    static_cast<TestableNetwork *>(testable[4])->Render_called = false;
    static_cast<TestableNetwork *>(testable[5])->Render_called = false;

    testable.ScrollUp();
    CHECK_TRUE(testable.Render(frame_buffer));
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[1])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[2])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[3])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[4])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[5])->Render_called);

    static_cast<TestableNetwork *>(testable[0])->Render_called = false;
    static_cast<TestableNetwork *>(testable[1])->Render_called = false;
    static_cast<TestableNetwork *>(testable[2])->Render_called = false;
    static_cast<TestableNetwork *>(testable[3])->Render_called = false;
    static_cast<TestableNetwork *>(testable[4])->Render_called = false;
    static_cast<TestableNetwork *>(testable[5])->Render_called = false;

    testable.ScrollUp();
    CHECK_TRUE(testable.Render(frame_buffer));
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[1])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[2])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[3])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[4])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[5])->Render_called);

    static_cast<TestableNetwork *>(testable[0])->Render_called = false;
    static_cast<TestableNetwork *>(testable[1])->Render_called = false;
    static_cast<TestableNetwork *>(testable[2])->Render_called = false;
    static_cast<TestableNetwork *>(testable[3])->Render_called = false;
    static_cast<TestableNetwork *>(testable[4])->Render_called = false;
    static_cast<TestableNetwork *>(testable[5])->Render_called = false;

    testable.ScrollUp();
    CHECK_TRUE(testable.Render(frame_buffer));
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[1])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[2])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[3])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[4])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[5])->Render_called);
}

TEST(LogicLadderTestsGroup, Render__when_networks_less_than_viewport) {
    TestableLadder testable;

    testable.Append(new TestableNetwork(LogicItemState::lisActive));
    testable.AutoScroll();

    CHECK_TRUE(testable.Render(frame_buffer));
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[0])->Render_called);

    testable.Append(new TestableNetwork(LogicItemState::lisActive));

    static_cast<TestableNetwork *>(testable[0])->Render_called = false;
    static_cast<TestableNetwork *>(testable[1])->Render_called = false;

    CHECK_TRUE(testable.Render(frame_buffer));
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[1])->Render_called);

    testable.Append(new TestableNetwork(LogicItemState::lisActive));

    static_cast<TestableNetwork *>(testable[0])->Render_called = false;
    static_cast<TestableNetwork *>(testable[1])->Render_called = false;
    static_cast<TestableNetwork *>(testable[2])->Render_called = false;

    CHECK_TRUE(testable.Render(frame_buffer));
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[1])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[2])->Render_called);
}

TEST(LogicLadderTestsGroup, CanScrollAuto_after_appending_second_network) {
    TestableLadder testable;

    CHECK_FALSE(testable.CanScrollAuto());
    testable.Append(new Network());

    CHECK_FALSE(testable.CanScrollAuto());
    testable.Append(new Network());

    CHECK_TRUE(testable.CanScrollAuto());
}

TEST(LogicLadderTestsGroup, AutoScroll_when_append_new_networks) {
    TestableLadder testable;

    testable.AutoScroll();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network());
    testable.AutoScroll();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network());
    testable.AutoScroll();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network());
    testable.AutoScroll();
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network());
    testable.AutoScroll();
    CHECK_EQUAL(2, *testable.PublicMorozov_Get_view_top_index());
}

TEST(LogicLadderTestsGroup, ScrollDown_ScrollUp_can_scroll_from_first_to_last_network) {
    TestableLadder testable;
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.Append(new Network(LogicItemState::lisActive));
    testable.AutoScroll();

    CHECK_EQUAL(5, *testable.PublicMorozov_Get_view_top_index());
    testable.ScrollDown();
    CHECK_EQUAL(5, *testable.PublicMorozov_Get_view_top_index());

    testable.ScrollUp();
    CHECK_EQUAL(4, *testable.PublicMorozov_Get_view_top_index());

    testable.ScrollUp();
    CHECK_EQUAL(3, *testable.PublicMorozov_Get_view_top_index());

    testable.ScrollUp();
    CHECK_EQUAL(2, *testable.PublicMorozov_Get_view_top_index());

    testable.ScrollUp();
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());

    testable.ScrollUp();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.ScrollUp();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.ScrollDown();
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());
}

TEST(LogicLadderTestsGroup, ScrollDown_ScrollUp__when_networks_less_than_viewport) {
    TestableLadder testable;
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.AutoScroll();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
    testable.ScrollDown();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
    testable.ScrollUp();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network(LogicItemState::lisActive));

    testable.AutoScroll();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
    testable.ScrollDown();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
    testable.ScrollUp();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network(LogicItemState::lisActive));

    testable.AutoScroll();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
    testable.ScrollDown();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
    testable.ScrollUp();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());

    testable.Append(new Network(LogicItemState::lisActive));

    testable.AutoScroll();
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());
    testable.ScrollDown();
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_view_top_index());
    testable.ScrollUp();
    CHECK_EQUAL(0, *testable.PublicMorozov_Get_view_top_index());
}