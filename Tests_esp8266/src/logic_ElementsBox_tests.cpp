#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "MonitorLogicElement.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/ElementsBox.cpp"
#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/LogicProgram.h"
#include "main/LogicProgram/Outputs/IncOutput.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicElementsBoxTestsGroup){ //
                                        TEST_SETUP(){ mock().disable();
memset(frame_buffer, 0, sizeof(frame_buffer));
}

TEST_TEARDOWN() {
    mock().enable();
}
}
;

namespace {
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
    };

} // namespace

TEST(LogicElementsBoxTestsGroup, box_for_inputs_elements) {
    InputNC stored_element(MapIO::V1);
    ElementsBox testable(100, &stored_element);
    CHECK_EQUAL(8, testable.size());
}

TEST(LogicElementsBoxTestsGroup, box_for_outputs_elements) {
    IncOutput stored_element(MapIO::O1);
    ElementsBox testable(100, &stored_element);
    CHECK_EQUAL(4, testable.size());
}

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_input_element) {
    InputNC stored_element(MapIO::V1);
    ElementsBox testable(100, &stored_element);
    CHECK_EQUAL(8, testable.size());
    for (auto *element : testable) {
        auto *element_as_commonInput = CommonInput::TryToCast(element);
        if (element_as_commonInput != NULL) {
            CHECK_EQUAL(MapIO::V1, element_as_commonInput->GetIoAdr());
        }
    }
}

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_output_element) {
    IncOutput stored_element(MapIO::O1);
    ElementsBox testable(100, &stored_element);
    CHECK_EQUAL(4, testable.size());
    for (auto *element : testable) {
        auto *element_as_commonOutput = CommonOutput::TryToCast(element);
        if (element_as_commonOutput != NULL) {
            CHECK_EQUAL(MapIO::O1, element_as_commonOutput->GetIoAdr());
        }
    }
}

TEST(LogicElementsBoxTestsGroup, no_available_place_for_timers_and_comparators) {
    InputNC stored_element(MapIO::V1);
    ElementsBox testable(30, &stored_element);
    CHECK_EQUAL(1, testable.size());
}

TEST(LogicElementsBoxTestsGroup, use_GetElementType_from_selected) {
    InputNC stored_element(MapIO::V1);
    ElementsBox testable(100, &stored_element);
    CHECK_EQUAL(TvElementType::et_InputNC, testable.GetElementType());
}

TEST(LogicElementsBoxTestsGroup, use_DoAction_from_selected) {
    TestableComparatorEq stored_element(42, MapIO::AI);
    ElementsBox testable(100, &stored_element);
    testable.DoAction(false, LogicItemState::lisActive);

    CHECK_TRUE(stored_element.DoAction_called);
}

TEST(LogicElementsBoxTestsGroup, use_Render_from_selected) {
    TestableComparatorEq stored_element(42, MapIO::AI);
    ElementsBox testable(100, &stored_element);
    Point start_point = {};
    testable.Render(frame_buffer, LogicItemState::lisActive, &start_point);

    CHECK_TRUE(stored_element.Render_called);
}

TEST(LogicElementsBoxTestsGroup, HandleButtonUp__change__selected_index__to_backward) {
    ComparatorEq stored_element(42, MapIO::AI);
    ElementsBox testable(100, &stored_element);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_ComparatorLs, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_ComparatorLE, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_ComparatorGr, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_ComparatorGE, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_TimerSecs, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_InputNO, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_InputNC, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
}

TEST(LogicElementsBoxTestsGroup, HandleButtonUp_selecting_elements_in_reverse_loop) {
    auto stored_element = new ComparatorEq(42, MapIO::AI);
    ElementsBox testable(100, stored_element);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_ComparatorLs, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_ComparatorLE, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_ComparatorGr, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_ComparatorGE, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_TimerSecs, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_InputNO, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_InputNC, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.HandleButtonUp();
    CHECK_EQUAL(TvElementType::et_ComparatorLs, testable.GetElementType());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, HandleButtonDown_selecting_elements_in_loop) {
    auto stored_element = new ComparatorEq(42, MapIO::AI);
    ElementsBox testable(100, stored_element);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.HandleButtonDown();
    CHECK_EQUAL(TvElementType::et_InputNC, testable.GetElementType());
    testable.HandleButtonDown();
    CHECK_EQUAL(TvElementType::et_InputNO, testable.GetElementType());
    testable.HandleButtonDown();
    CHECK_EQUAL(TvElementType::et_TimerSecs, testable.GetElementType());
    testable.HandleButtonDown();
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable.GetElementType());
    testable.HandleButtonDown();
    CHECK_EQUAL(TvElementType::et_ComparatorGE, testable.GetElementType());
    testable.HandleButtonDown();
    CHECK_EQUAL(TvElementType::et_ComparatorGr, testable.GetElementType());
    testable.HandleButtonDown();
    CHECK_EQUAL(TvElementType::et_ComparatorLE, testable.GetElementType());
    testable.HandleButtonDown();
    CHECK_EQUAL(TvElementType::et_ComparatorLs, testable.GetElementType());
    testable.HandleButtonDown();
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.HandleButtonDown();
    CHECK_EQUAL(TvElementType::et_InputNC, testable.GetElementType());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, HandleButtonSelect_first_call_switch_element_to_editing) {
    ComparatorEq stored_element(42, MapIO::AI);
    ElementsBox testable(100, &stored_element);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.HandleButtonSelect();
    CHECK_TRUE(stored_element.Editing());
}

TEST(LogicElementsBoxTestsGroup, No_memleak_if_selection_changes) {
    auto stored_element = new ComparatorEq(42, MapIO::AI);
    ElementsBox testable(100, stored_element);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.HandleButtonDown();
    CHECK_EQUAL(TvElementType::et_InputNC, testable.GetElementType());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, No_memleak_if_no_selection_changes) {
    auto stored_element = new ComparatorEq(42, MapIO::AI);
    ElementsBox testable(100, stored_element);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, In_editing_no_memleak_if_selection_changes) {
    auto stored_element = new ComparatorEq(42, MapIO::AI);
    ElementsBox testable(100, stored_element);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.Select();
    testable.BeginEditing();
    testable.HandleButtonDown();
    CHECK_EQUAL(TvElementType::et_InputNC, testable.GetElementType());
}

TEST(LogicElementsBoxTestsGroup, In_editing_no_memleak_if_no_selection_changes) {
    auto stored_element = new ComparatorEq(42, MapIO::AI);
    ElementsBox testable(100, stored_element);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.Select();
    testable.BeginEditing();
}