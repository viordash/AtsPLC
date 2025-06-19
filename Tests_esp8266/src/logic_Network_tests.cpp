#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "MonitorLogicElement.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "LogicProgram/Bindings/WiFiBinding.h"
#include "main/LogicProgram/ElementsBox.h"
#include "main/LogicProgram/LogicProgram.h"
#include "main/LogicProgram/Network.h"

static uint8_t frame_buffer[DISPLAY_HEIGHT_IN_BYTES * DISPLAY_WIDTH] = {};

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
        TestableNetwork(LogicItemState state) : Network(state) {
        }
        LogicItemState PublicMorozov_state() {
            return state;
        }
        uint8_t *PublicMorozov_Get_fill_wire() {
            return &fill_wire;
        }
        bool PublicMorozov_Get_state_changed() {
            return state_changed;
        }
        void PublicMorozov_AddSpaceForNewElement() {
            AddSpaceForNewElement();
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
            return MonitorLogicElement::Render(start_point);
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
            return MonitorLogicElement::Render(start_point);
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
            return MonitorLogicElement::Render(start_point);
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
            return MonitorLogicElement::Render(start_point);
        }
    };

    class TestableWire : public Wire, public MonitorLogicElement {
      public:
        explicit TestableWire() : Wire() {
        }
        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            return MonitorLogicElement::DoAction();
        }

        bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            return MonitorLogicElement::Render(start_point);
        }
    };

    class TestableIndicator : public Indicator, public MonitorLogicElement {
      public:
        explicit TestableIndicator() : Indicator() {
        }
        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            return MonitorLogicElement::DoAction();
        }

        bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            return MonitorLogicElement::Render(start_point);
        }
    };

    class TestableWiFiBinding : public WiFiBinding, public MonitorLogicElement {
      public:
        explicit TestableWiFiBinding() : WiFiBinding() {
        }
        bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
            (void)prev_changed;
            (void)prev_elem_state;
            return MonitorLogicElement::DoAction();
        }

        bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            return MonitorLogicElement::Render(start_point);
        }
    };
} // namespace

TEST(LogicNetworkTestsGroup, append_elements) {
    Network testable(LogicItemState::lisActive);

    testable.Append(new TestableInputNC);
    testable.Append(new TestableComparatorEq());
    testable.Append(new TestableTimerMSecs());
    testable.Append(new TestableDirectOutput);

    CHECK_EQUAL(4, testable.size());
}

TEST(LogicNetworkTestsGroup, DoAction_handle_all_logic_elements_in_chain) {
    Network testable(LogicItemState::lisActive);

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
    Network testable(LogicItemState::lisActive);

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
    Network testable(LogicItemState::lisActive);

    testable.Append(new TestableInputNC);
    testable.Append(new TestableComparatorEq());
    testable.Append(new TestableTimerMSecs());
    testable.Append(new TestableDirectOutput);

    CHECK_TRUE(testable.Render(frame_buffer, 0));

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

TEST(LogicNetworkTestsGroup, Render_with_Indicator_element) {
    Network testable(LogicItemState::lisActive);

    testable.Append(new TestableIndicator);
    CHECK_TRUE(testable.Render(frame_buffer, 0));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_TRUE(static_cast<TestableIndicator *>(testable[0])->Render_called);
}

TEST(LogicNetworkTestsGroup, Render_with_Wire_element) {
    Network testable(LogicItemState::lisActive);

    testable.Append(new TestableWire);
    CHECK_TRUE(testable.Render(frame_buffer, 0));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_TRUE(static_cast<TestableWire *>(testable[0])->Render_called);
}

TEST(LogicNetworkTestsGroup, Render_with_WiFiBinding_element) {
    Network testable(LogicItemState::lisActive);

    testable.Append(new TestableWiFiBinding);
    CHECK_TRUE(testable.Render(frame_buffer, 0));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_TRUE(static_cast<TestableWiFiBinding *>(testable[0])->Render_called);
}

TEST(LogicNetworkTestsGroup, Render_when_passive__also_render_all_elements_in_chain) {
    Network testable(LogicItemState::lisActive);

    testable.Append(new TestableInputNC);
    testable.Append(new TestableComparatorEq());
    testable.Append(new TestableTimerMSecs());
    testable.Append(new TestableDirectOutput);

    CHECK_TRUE(testable.Render(frame_buffer, 0));

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
    Network testable(LogicItemState::lisActive);

    testable.Append(new TestableInputNC);
    testable.Append(new TestableComparatorEq());
    testable.Append(new TestableTimerMSecs());
    testable.Append(new TestableDirectOutput);
    static_cast<TestableComparatorEq *>(testable[1])->Render_result = false;

    CHECK_FALSE(testable.Render(frame_buffer, 0));

    CHECK_TRUE(static_cast<TestableInputNC *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableComparatorEq *>(testable[1])->Render_called);
    CHECK_FALSE(static_cast<TestableTimerMSecs *>(testable[2])->Render_called);
    CHECK_FALSE(static_cast<TestableDirectOutput *>(testable[3])->Render_called);
}

TEST(LogicNetworkTestsGroup,
     Render_inputs_starts_from_start_point_and_render_outputs_starts_from_end) {
    Network testable(LogicItemState::lisActive);

    testable.Append(new TestableInputNC);
    testable.Append(new TestableComparatorEq());
    testable.Append(new TestableTimerMSecs());
    testable.Append(new TestableDirectOutput);

    CHECK_TRUE(testable.Render(frame_buffer, 0));

    CHECK_TRUE(static_cast<TestableInputNC *>(testable[0])->Render_called);
    CHECK_TRUE(static_cast<TestableComparatorEq *>(testable[1])->Render_called);
    CHECK_TRUE(static_cast<TestableTimerMSecs *>(testable[2])->Render_called);
    CHECK_TRUE(static_cast<TestableDirectOutput *>(testable[3])->Render_called);

    CHECK_EQUAL(INCOME_RAIL_WIDTH,
                static_cast<TestableInputNC *>(testable[0])->Render_start_point.x);
    CHECK_EQUAL(INCOME_RAIL_WIDTH,
                static_cast<TestableComparatorEq *>(testable[1])->Render_start_point.x);
    CHECK_EQUAL(INCOME_RAIL_WIDTH,
                static_cast<TestableTimerMSecs *>(testable[2])->Render_start_point.x);
    CHECK_EQUAL(OUTCOME_RAIL_RIGHT,
                static_cast<TestableDirectOutput *>(testable[3])->Render_start_point.x);
}

TEST(LogicNetworkTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    Network testable(LogicItemState::lisActive);
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

TEST(LogicNetworkTestsGroup, Serialize_Indicator) {
    uint8_t buffer[256] = {};
    Network testable(LogicItemState::lisActive);
    auto indicator = new Indicator(MapIO::AI);
    testable.Append(indicator);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(14, writed);

    CHECK_EQUAL(LogicItemState::lisActive, *((LogicItemState *)&buffer[0]));
    CHECK_EQUAL(1, *((uint16_t *)&buffer[1]));

    CHECK_EQUAL(TvElementType::et_Indicator, *((TvElementType *)&buffer[3]));
    CHECK_EQUAL(MapIO::AI, *((MapIO *)&buffer[4]));
    DOUBLES_EQUAL(0.0, *((float *)&buffer[5]), 0.0001);
    DOUBLES_EQUAL(100.0, *((float *)&buffer[9]), 0.0001);
    CHECK_EQUAL(2, *((uint8_t *)&buffer[13]));
}

TEST(LogicNetworkTestsGroup, Serialize_just_for_obtain_size) {
    Network testable(LogicItemState::lisActive);
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
    Network testable(LogicItemState::lisActive);

    for (size_t i = 0; i < 6; i++) {
        testable.Append(new TestableInputNC);
    }

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicNetworkTestsGroup, Serialize_when_elemens_count_less_than_min__return_zero) {
    uint8_t buffer[256] = {};
    Network testable(LogicItemState::lisActive);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicNetworkTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[256] = {};
    Network testable(LogicItemState::lisActive);
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

    Network testable(LogicItemState::lisActive);

    size_t readed = testable.Deserialize(&buffer[0], sizeof(buffer) - 1);
    CHECK_EQUAL(19, readed);
    CHECK_EQUAL(4, testable.size());
    CHECK_EQUAL(TvElementType::et_InputNC, testable[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable[1]->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable[2]->GetElementType());
    CHECK_EQUAL(TvElementType::et_DirectOutput, testable[3]->GetElementType());
}

TEST(LogicNetworkTestsGroup, Begin_Editing_and_replacing_selected_element_with_ElementBox) {
    Network testable(LogicItemState::lisActive);

    testable.Append(new InputNC(MapIO::DI));
    testable.Append(new ComparatorEq(1, MapIO::AI));

    testable.SelectNext();

    auto selectedElement = testable[testable.GetSelectedElement()];
    CHECK_EQUAL(TvElementType::et_InputNC, selectedElement->GetElementType());

    testable.Change();

    auto expectedElementBox = testable[testable.GetSelectedElement()];
    CHECK_EQUAL(TvElementType::et_InputNC, expectedElementBox->GetElementType());
    CHECK_TRUE(expectedElementBox->Editing());
    CHECK(selectedElement != expectedElementBox);
    auto elementBox = static_cast<ElementsBox *>(expectedElementBox);
    delete elementBox->GetSelectedElement();
}

TEST(LogicNetworkTestsGroup, Begin_Editing_can_hide_output_elements_in_ElementBox) {
    Network testable(LogicItemState::lisActive);

    testable.Append(new InputNC(MapIO::DI));
    testable.Append(new DirectOutput(MapIO::O1));
    CHECK_TRUE(testable.Render(frame_buffer, 0));

    testable.SelectNext();

    auto selectedElement = testable[testable.GetSelectedElement()];
    CHECK_EQUAL(TvElementType::et_InputNC, selectedElement->GetElementType());

    testable.Change();

    auto expectedElementBox = testable[testable.GetSelectedElement()];
    CHECK_EQUAL(TvElementType::et_InputNC, expectedElementBox->GetElementType());
    CHECK_TRUE(expectedElementBox->Editing());
    auto elementBox = static_cast<ElementsBox *>(expectedElementBox);
    CHECK_EQUAL(11, elementBox->size());

    testable.SelectNext();
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_ComparatorLs, expectedElementBox->GetElementType());

    delete elementBox->GetSelectedElement();
}

TEST(LogicNetworkTestsGroup,
     Begin_Editing_not_hide_output_elements_in_ElementBox_if_selected_element_is_output) {
    Network testable(LogicItemState::lisActive);

    testable.Append(new InputNC(MapIO::DI));
    testable.Append(new DirectOutput(MapIO::O1));
    CHECK_TRUE(testable.Render(frame_buffer, 0));

    testable.SelectNext();
    testable.SelectNext();

    auto selectedElement = testable[testable.GetSelectedElement()];
    CHECK_EQUAL(TvElementType::et_DirectOutput, selectedElement->GetElementType());

    testable.Change();

    auto expectedElementBox = testable[testable.GetSelectedElement()];
    CHECK_EQUAL(TvElementType::et_DirectOutput, expectedElementBox->GetElementType());
    CHECK_TRUE(expectedElementBox->Editing());
    auto elementBox = static_cast<ElementsBox *>(expectedElementBox);
    CHECK_EQUAL(16, elementBox->size());

    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_SetOutput, expectedElementBox->GetElementType());

    delete elementBox->GetSelectedElement();
}

TEST(LogicNetworkTestsGroup, EndEditing_ElementBox_switch_selection_to_network_self) {
    Network testable(LogicItemState::lisActive);

    testable.Append(new InputNC(MapIO::DI));
    testable.Append(new ComparatorEq(1, MapIO::AI));

    testable.SelectNext();

    auto selectedElement = testable[testable.GetSelectedElement()];
    CHECK_EQUAL(TvElementType::et_InputNC, selectedElement->GetElementType());

    testable.Change();

    auto expectedElementBox = testable[testable.GetSelectedElement()];
    CHECK_EQUAL(TvElementType::et_InputNC, expectedElementBox->GetElementType());
    CHECK(selectedElement != expectedElementBox);
    auto elementBox = static_cast<ElementsBox *>(expectedElementBox);

    testable.Change();
    CHECK_TRUE(elementBox->GetSelectedElement()->Editing());

    testable.Change();

    CHECK_EQUAL(-1, testable.GetSelectedElement());
}

TEST(LogicNetworkTestsGroup, SwitchState) {
    TestableNetwork testable(LogicItemState::lisActive);

    testable.SwitchState();
    CHECK_EQUAL(LogicItemState::lisPassive, testable.PublicMorozov_state());

    testable.SwitchState();
    CHECK_EQUAL(LogicItemState::lisActive, testable.PublicMorozov_state());
}

TEST(LogicNetworkTestsGroup, ChangeState_updates_state_in_child_elements) {
    TestableNetwork testable(LogicItemState::lisActive);

    testable.ChangeState(LogicItemState::lisPassive);
    CHECK_TRUE(testable.PublicMorozov_Get_state_changed());
    testable.DoAction();
    CHECK_FALSE(testable.PublicMorozov_Get_state_changed());

    testable.ChangeState(LogicItemState::lisActive);
    CHECK_TRUE(testable.PublicMorozov_Get_state_changed());
    testable.DoAction();
    CHECK_FALSE(testable.PublicMorozov_Get_state_changed());
}

TEST(LogicNetworkTestsGroup, when_no_free_place_then_cannot_add_new_element) {
    TestableNetwork testable(LogicItemState::lisActive);

    testable.Append(new InputNC(MapIO::DI));
    testable.Append(new ComparatorEq(1, MapIO::AI));
    testable.Append(new TimerMSecs(100));
    testable.Append(new DirectOutput(MapIO::O1));

    CHECK_TRUE(testable.Render(frame_buffer, 0));

    testable.BeginEditing();
    CHECK_EQUAL(4, testable.size());
}

TEST(LogicNetworkTestsGroup, ability_to_add_new_element) {
    TestableNetwork testable(LogicItemState::lisActive);

    testable.Append(new InputNC(MapIO::DI));
    testable.Append(new DirectOutput(MapIO::O1));

    CHECK_TRUE(testable.Render(frame_buffer, 0));

    testable.BeginEditing();
    CHECK_EQUAL(3, testable.size());
    CHECK_EQUAL(TvElementType::et_Wire, testable[1]->GetElementType());
}

TEST(LogicNetworkTestsGroup, wire_element__take__all__empty_space) {
    TestableNetwork testable(LogicItemState::lisActive);

    testable.Append(new InputNC(MapIO::DI));
    testable.Append(new DirectOutput(MapIO::O1));

    CHECK_TRUE(testable.Render(frame_buffer, 0));
    CHECK_EQUAL(71, *testable.PublicMorozov_Get_fill_wire());

    testable.BeginEditing();
    CHECK_EQUAL(3, testable.size());

    CHECK_TRUE(testable.Render(frame_buffer, 0));

    CHECK_EQUAL(47, *testable.PublicMorozov_Get_fill_wire());
}

TEST(LogicNetworkTestsGroup, Wire_elements_must_be_deleted_after_editing) {
    Network testable(LogicItemState::lisActive);

    testable.Append(new InputNC(MapIO::DI));
    testable.Append(new ComparatorEq(1, MapIO::AI));
    testable.Append(new DirectOutput(MapIO::O1));

    CHECK_TRUE(testable.Render(frame_buffer, 0));
    testable.BeginEditing();
    CHECK_EQUAL(4, testable.size());

    testable.SelectNext();

    auto selectedElement = testable[testable.GetSelectedElement()];
    CHECK_EQUAL(TvElementType::et_InputNC, selectedElement->GetElementType());

    testable.Change();

    auto expectedElementBox = testable[testable.GetSelectedElement()];
    CHECK_EQUAL(TvElementType::et_InputNC, expectedElementBox->GetElementType());
    CHECK(selectedElement != expectedElementBox);
    auto elementBox = static_cast<ElementsBox *>(expectedElementBox);

    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_Wire, elementBox->GetSelectedElement()->GetElementType());

    testable.Change();
    CHECK_EQUAL(-1, testable.GetSelectedElement());

    CHECK_EQUAL(3, testable.size());
    testable.Change();
    CHECK_EQUAL(2, testable.size());
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable[0]->GetElementType());
}

TEST(LogicNetworkTestsGroup, EndEditing_delete_ElementBox) {
    Network testable(LogicItemState::lisActive);

    testable.Append(new InputNC(MapIO::DI));
    testable.Append(new ComparatorEq(1, MapIO::AI));
    CHECK_TRUE(testable.Render(frame_buffer, 0));

    testable.SelectNext();

    auto selectedElement = testable[testable.GetSelectedElement()];
    CHECK_EQUAL(TvElementType::et_InputNC, selectedElement->GetElementType());

    testable.Change();

    auto expectedElementBox = testable[testable.GetSelectedElement()];
    CHECK_EQUAL(TvElementType::et_InputNC, expectedElementBox->GetElementType());
    CHECK(selectedElement != expectedElementBox);
    auto elementBox = static_cast<ElementsBox *>(expectedElementBox);

    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_InputNO, expectedElementBox->GetElementType());

    testable.Change();
    CHECK_TRUE(elementBox->GetSelectedElement()->Editing());

    testable.Change();

    CHECK_EQUAL(-1, testable.GetSelectedElement());
}

TEST(LogicNetworkTestsGroup, Space_For_New_Element_Is_Placed_After_Input) {
    TestableNetwork testable(LogicItemState::lisActive);
    *testable.PublicMorozov_Get_fill_wire() = 100;
    testable.Append(new InputNC(MapIO::DI));

    testable.PublicMorozov_AddSpaceForNewElement();

    CHECK_EQUAL(2, testable.size());
    CHECK_EQUAL(TvElementType::et_InputNC, testable[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_Wire, testable[1]->GetElementType());
}

TEST(LogicNetworkTestsGroup, Space_For_New_Element_Is_Placed_After_Timer) {
    TestableNetwork testable(LogicItemState::lisActive);
    *testable.PublicMorozov_Get_fill_wire() = 100;
    testable.Append(new TimerSecs(100));

    testable.PublicMorozov_AddSpaceForNewElement();

    CHECK_EQUAL(2, testable.size());
    CHECK_EQUAL(TvElementType::et_TimerSecs, testable[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_Wire, testable[1]->GetElementType());
}

TEST(LogicNetworkTestsGroup, Space_For_New_Element_Is_Placed_After_Comparator) {
    TestableNetwork testable(LogicItemState::lisActive);
    *testable.PublicMorozov_Get_fill_wire() = 100;
    testable.Append(new ComparatorEq(10, MapIO::AI));

    testable.PublicMorozov_AddSpaceForNewElement();

    CHECK_EQUAL(2, testable.size());
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_Wire, testable[1]->GetElementType());
}

TEST(LogicNetworkTestsGroup, Space_For_New_Element_Is_Placed_After_Indicator) {
    TestableNetwork testable(LogicItemState::lisActive);
    *testable.PublicMorozov_Get_fill_wire() = 100;
    testable.Append(new Indicator(MapIO::AI));

    testable.PublicMorozov_AddSpaceForNewElement();

    CHECK_EQUAL(2, testable.size());
    CHECK_EQUAL(TvElementType::et_Indicator, testable[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_Wire, testable[1]->GetElementType());
}

TEST(LogicNetworkTestsGroup, Space_For_New_Element_Is_Placed_After_WiFiBinding) {
    TestableNetwork testable(LogicItemState::lisActive);
    *testable.PublicMorozov_Get_fill_wire() = 100;
    testable.Append(new WiFiBinding(MapIO::AI, "ssid"));

    testable.PublicMorozov_AddSpaceForNewElement();

    CHECK_EQUAL(2, testable.size());
    CHECK_EQUAL(TvElementType::et_WiFiBinding, testable[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_Wire, testable[1]->GetElementType());
}

TEST(LogicNetworkTestsGroup, Space_For_New_Element_Is_Placed_After_DateTimeBinding) {
    TestableNetwork testable(LogicItemState::lisActive);
    *testable.PublicMorozov_Get_fill_wire() = 100;
    testable.Append(new DateTimeBinding(MapIO::AI));

    testable.PublicMorozov_AddSpaceForNewElement();

    CHECK_EQUAL(2, testable.size());
    CHECK_EQUAL(TvElementType::et_DateTimeBinding, testable[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_Wire, testable[1]->GetElementType());
}

TEST(LogicNetworkTestsGroup, Space_For_New_Element_Is_Placed_After_Settings) {
    TestableNetwork testable(LogicItemState::lisActive);
    *testable.PublicMorozov_Get_fill_wire() = 100;
    testable.Append(new SettingsElement());

    testable.PublicMorozov_AddSpaceForNewElement();

    CHECK_EQUAL(2, testable.size());
    CHECK_EQUAL(TvElementType::et_Settings, testable[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_Wire, testable[1]->GetElementType());
}