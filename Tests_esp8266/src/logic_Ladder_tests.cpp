#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "MonitorLogicElement.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Ladder.h"
#include "main/LogicProgram/LogicProgram.h"
#include "main/redundant_storage.h"

#include "tests_utils.h"

static FrameBuffer frame_buffer = {};

TEST_GROUP(LogicLadderTestsGroup){ //
                                   TEST_SETUP(){ mock().disable();
memset(&frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));
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

    class TestableLadder : public Ladder {
      public:
        TestableLadder() : Ladder() {
        }
    };

    class TestableNetwork : public Network, public MonitorLogicElement {
      public:
        TestableNetwork(LogicItemState state) : Network(state) {
        }

        bool DoAction() override {
            return MonitorLogicElement::DoAction();
        }

        Point start_point = {};

        void Render(FrameBuffer *fb, uint8_t network_number) override {
            (void)fb;
            (void)network_number;
            return MonitorLogicElement::Render(&start_point);
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

        void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            MonitorLogicElement::Render(start_point);
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

        void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            MonitorLogicElement::Render(start_point);
        }
        uint8_t PublicMorozov_GetReference() {
            return ref_percent04;
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

        void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            MonitorLogicElement::Render(start_point);
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

        void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            MonitorLogicElement::Render(start_point);
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
    CHECK_EQUAL(MapIO::DI, ((TestableInputNC *)(*network_load)[0])->GetIoAdr());
    CHECK(&Controller::DI == ((TestableInputNC *)(*network_load)[0])->Input);

    CHECK_EQUAL(TvElementType::et_ComparatorEq, (*network_load)[1]->GetElementType());
    CHECK_EQUAL(5, ((TestableComparatorEq *)(*network_load)[1])->PublicMorozov_GetReference());
    CHECK_EQUAL(MapIO::AI, ((TestableComparatorEq *)(*network_load)[1])->GetIoAdr());
    CHECK(&Controller::AI == ((TestableComparatorEq *)(*network_load)[1])->Input);

    CHECK_EQUAL(TvElementType::et_TimerMSecs, (*network_load)[2]->GetElementType());
    CHECK_EQUAL(12345000,
                ((TestableTimerMSecs *)(*network_load)[2])->PublicMorozov_GetDelayTimeUs());

    CHECK_EQUAL(TvElementType::et_DirectOutput, (*network_load)[3]->GetElementType());
    CHECK_EQUAL(MapIO::O1, ((TestableDirectOutput *)(*network_load)[3])->GetIoAdr());
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

    CHECK_EQUAL(0, ladder_load.size());
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
    CHECK_EQUAL(0, ladder_load.size());
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

    testable.Render(&frame_buffer);
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

    testable.HandleButtonUp();
    testable.Render(&frame_buffer);
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

    testable.HandleButtonUp();
    testable.Render(&frame_buffer);
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

    testable.HandleButtonUp();
    testable.Render(&frame_buffer);
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

    testable.HandleButtonUp();
    testable.Render(&frame_buffer);
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

    testable.HandleButtonUp();
    testable.Render(&frame_buffer);
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

    testable.Render(&frame_buffer);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[0])->Render_called);

    testable.Append(new TestableNetwork(LogicItemState::lisActive));

    static_cast<TestableNetwork *>(testable[0])->Render_called = false;
    static_cast<TestableNetwork *>(testable[1])->Render_called = false;

    testable.Render(&frame_buffer);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[1])->Render_called);

    testable.Append(new TestableNetwork(LogicItemState::lisActive));

    static_cast<TestableNetwork *>(testable[0])->Render_called = false;
    static_cast<TestableNetwork *>(testable[1])->Render_called = false;
    static_cast<TestableNetwork *>(testable[2])->Render_called = false;

    testable.Render(&frame_buffer);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableNetwork *>(testable[1])->Render_called);
    CHECK_FALSE(static_cast<TestableNetwork *>(testable[2])->Render_called);
}

TEST(LogicLadderTestsGroup, AtLeastOneNetwork__if_no_networks_then_create_ones_and_show_designer) {
    TestableLadder testable;

    testable.AtLeastOneNetwork();

    CHECK_EQUAL(1, testable.size());
    CHECK_TRUE(testable[0]->Selected());
}

TEST(LogicLadderTestsGroup, Delete_storage) {
    Ladder ladder_store;

    auto network_store = new Network(LogicItemState::lisActive);
    ladder_store.Append(network_store);

    network_store->Append(new TestableInputNC(MapIO::DI));
    network_store->Append(new TestableComparatorEq(5, MapIO::AI));
    network_store->Append(new TestableTimerMSecs(12345));
    network_store->Append(new TestableDirectOutput(MapIO::O1));
    ladder_store.Store();

    Ladder::DeleteStorage();

    Ladder ladder_load;
    ladder_load.Load();

    CHECK_EQUAL(0, ladder_load.size());
}

TEST(LogicLadderTestsGroup, Delete_network) {
    TestableLadder testable;
    auto network0 = new Network(LogicItemState::lisActive);
    auto network1 = new Network(LogicItemState::lisActive);
    auto network2 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    network1->Append(new InputNC(MapIO::DI));
    network2->Append(new InputNC(MapIO::DI));
    testable.Append(network0);
    testable.Append(network1);
    testable.Append(network2);

    testable.Delete(1);
    CHECK_EQUAL(2, testable.size());
    CHECK_EQUAL(testable[0], network0);
    CHECK_EQUAL(testable[1], network2);

    testable.Delete(1);
    CHECK_EQUAL(1, testable.size());
    CHECK_EQUAL(testable[0], network0);

    testable.Delete(0);
    CHECK_EQUAL(0, testable.size());
}

TEST(LogicLadderTestsGroup, Duplicate_network) {
    TestableLadder testable;
    auto network0 = new Network(LogicItemState::lisActive);
    auto network1 = new Network(LogicItemState::lisActive);
    network0->Append(new InputNC(MapIO::DI));
    network1->Append(new InputNC(MapIO::DI));
    testable.Append(network0);
    testable.Append(network1);

    testable.Duplicate(1);
    CHECK_EQUAL(3, testable.size());
    CHECK_EQUAL(testable[0], network0);
    CHECK(testable[1] != network0);
    CHECK(testable[1] != network1);
    CHECK_EQUAL(testable[2], network1);

    testable.Duplicate(0);
    CHECK_EQUAL(4, testable.size());
    CHECK(testable[0] != network0);
    CHECK(testable[0] != network1);
    CHECK_EQUAL(testable[1], network0);
    CHECK(testable[2] != network0);
    CHECK(testable[2] != network1);
    CHECK_EQUAL(testable[3], network1);
}
