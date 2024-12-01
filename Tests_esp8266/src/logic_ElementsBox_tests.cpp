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
#include "main/LogicProgram/Inputs/ComparatorEq.h"
#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/LogicProgram.h"
#include "main/LogicProgram/Outputs/IncOutput.h"
#include "main/LogicProgram/Wire.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicElementsBoxTestsGroup){ //
                                        TEST_SETUP(){ mock().disable();
memset(frame_buffer, 0, sizeof(frame_buffer));
Controller::Start(NULL);
}

TEST_TEARDOWN() {
    Controller::Stop();
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

TEST(LogicElementsBoxTestsGroup, box_fill_elements) {
    InputNC stored_element(MapIO::V1);
    ElementsBox testable(DISPLAY_WIDTH - INCOME_RAIL_WIDTH - SCROLLBAR_WIDTH,
                         &stored_element,
                         false);
    CHECK_EQUAL(17, testable.size());
    CHECK_EQUAL(TvElementType::et_InputNC, testable[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_InputNO, testable[1]->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerSecs, testable[2]->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable[3]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable[4]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorGE, testable[5]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorGr, testable[6]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorLE, testable[7]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorLs, testable[8]->GetElementType());
    CHECK_EQUAL(TvElementType::et_Indicator, testable[9]->GetElementType());
    CHECK_EQUAL(TvElementType::et_WiFiBinding, testable[10]->GetElementType());
    CHECK_EQUAL(TvElementType::et_DirectOutput, testable[11]->GetElementType());
    CHECK_EQUAL(TvElementType::et_SetOutput, testable[12]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ResetOutput, testable[13]->GetElementType());
    CHECK_EQUAL(TvElementType::et_IncOutput, testable[14]->GetElementType());
    CHECK_EQUAL(TvElementType::et_DecOutput, testable[15]->GetElementType());
    CHECK_EQUAL(TvElementType::et_Wire, testable[16]->GetElementType());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, hide_output_elements) {
    Wire stored_element;
    ElementsBox testable(DISPLAY_WIDTH - INCOME_RAIL_WIDTH - SCROLLBAR_WIDTH,
                         &stored_element,
                         true);
    CHECK_EQUAL(12, testable.size());
    CHECK_EQUAL(TvElementType::et_InputNC, testable[0]->GetElementType());
    CHECK_EQUAL(TvElementType::et_InputNO, testable[1]->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerSecs, testable[2]->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable[3]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable[4]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorGE, testable[5]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorGr, testable[6]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorLE, testable[7]->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorLs, testable[8]->GetElementType());
    CHECK_EQUAL(TvElementType::et_Indicator, testable[9]->GetElementType());
    CHECK_EQUAL(TvElementType::et_WiFiBinding, testable[10]->GetElementType());
    CHECK_EQUAL(TvElementType::et_Wire, testable[11]->GetElementType());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_input_element) {
    int matched = 0;
    InputNC stored_element(MapIO::V2);
    ElementsBox testable(100, &stored_element, false);
    for (auto *element : testable) {
        auto *element_as_commonInput = CommonInput::TryToCast(element);
        if (element_as_commonInput != NULL) {
            CHECK_EQUAL(MapIO::V2, element_as_commonInput->GetIoAdr());
            matched++;
        }
    }
    delete testable.GetSelectedElement();
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_comparator_element) {
    int matched = 0;
    ComparatorEq stored_element(42, MapIO::AI);
    ElementsBox testable(100, &stored_element, false);
    for (auto *element : testable) {
        auto *element_as_commonInput = CommonInput::TryToCast(element);
        if (element_as_commonInput != NULL) {
            auto *element_as_commonComparator = CommonComparator::TryToCast(element_as_commonInput);
            if (element_as_commonComparator != NULL) {
                CHECK_EQUAL(MapIO::AI, element_as_commonComparator->GetIoAdr());
                CHECK_EQUAL(42, element_as_commonComparator->GetReference());
                matched++;
            }
        }
    }
    delete testable.GetSelectedElement();
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup,
     takes_params_from_stored_element__set_reference_to_zero_by_default_for_comparators) {
    int matched = 0;
    InputNC stored_element(MapIO::V1);
    ElementsBox testable(100, &stored_element, false);
    for (auto *element : testable) {
        auto *element_as_commonInput = CommonInput::TryToCast(element);
        if (element_as_commonInput != NULL) {
            auto *element_as_commonComparator = CommonComparator::TryToCast(element_as_commonInput);
            if (element_as_commonComparator != NULL) {
                CHECK_EQUAL(MapIO::V1, element_as_commonComparator->GetIoAdr());
                CHECK_EQUAL(0, element_as_commonComparator->GetReference());
                matched++;
            }
        }
    }
    delete testable.GetSelectedElement();
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_TimerSec_element) {
    int matched = 0;
    TimerSecs stored_element(42);
    ElementsBox testable(100, &stored_element, false);
    for (auto *element : testable) {
        auto *element_as_commonTimer = CommonTimer::TryToCast(element);
        if (element_as_commonTimer != NULL) {
            auto *element_as_TimerMSecs = TimerMSecs::TryToCast(element_as_commonTimer);
            if (element_as_TimerMSecs != NULL) {
                CHECK_EQUAL(42 * 1000000LL, element_as_TimerMSecs->GetTimeUs());
                matched++;
            }
        }
    }
    delete testable.GetSelectedElement();
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_TimerMSec_element) {
    int matched = 0;
    TimerMSecs stored_element(42000);
    ElementsBox testable(100, &stored_element, false);
    for (auto *element : testable) {
        auto *element_as_commonTimer = CommonTimer::TryToCast(element);
        if (element_as_commonTimer != NULL) {
            auto *element_as_TimerSecs = TimerSecs::TryToCast(element_as_commonTimer);
            if (element_as_TimerSecs != NULL) {
                CHECK_EQUAL(42 * 1000000LL, element_as_TimerSecs->GetTimeUs());
                matched++;
            }
        }
    }
    delete testable.GetSelectedElement();
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup,
     takes_params_from_stored_element__set_default_delaytime_for_timers) {
    int matched = 0;
    InputNC stored_element(MapIO::V1);
    ElementsBox testable(100, &stored_element, false);
    for (auto *element : testable) {
        auto *element_as_commonTimer = CommonTimer::TryToCast(element);
        if (element_as_commonTimer != NULL) {
            auto *element_as_TimerSecs = TimerSecs::TryToCast(element_as_commonTimer);
            if (element_as_TimerSecs != NULL) {
                CHECK_EQUAL(1000000LL, element_as_TimerSecs->GetTimeUs());
                matched++;
            }

            auto *element_as_TimerMSecs = TimerMSecs::TryToCast(element_as_commonTimer);
            if (element_as_TimerMSecs != NULL) {
                CHECK_EQUAL(1000LL, element_as_TimerMSecs->GetTimeUs());
                matched++;
            }
        }
    }
    delete testable.GetSelectedElement();
    CHECK_COMPARE(matched, >, 1);
}

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_output_element) {
    int matched = 0;
    IncOutput stored_element(MapIO::O1);
    ElementsBox testable(100, &stored_element, false);
    for (auto *element : testable) {
        auto *element_as_commonOutput = CommonOutput::TryToCast(element);
        if (element_as_commonOutput != NULL) {
            CHECK_EQUAL(MapIO::O1, element_as_commonOutput->GetIoAdr());
            matched++;
        }
    }
    delete testable.GetSelectedElement();
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, takes_params_for_wire) {
    int matched = 0;
    InputNC stored_element(MapIO::V1);
    ElementsBox testable(100, &stored_element, false);
    for (auto *element : testable) {
        auto *element_as_wire = Wire::TryToCast(element);
        if (element_as_wire != NULL) {
            CHECK_EQUAL(24, element_as_wire->GetWidth());
            matched++;
        }
    }
    delete testable.GetSelectedElement();
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, copy_params_for_indicator_element) {
    int matched = 0;
    Indicator stored_element(MapIO::AI);
    stored_element.SetHighScale(42);
    stored_element.SetLowScale(19);
    stored_element.SetDecimalPoint(2);
    ElementsBox testable(100, &stored_element, false);
    for (auto *element : testable) {
        auto *element_as_indicator = Indicator::TryToCast(element);
        if (element_as_indicator != NULL) {
            CHECK_EQUAL(MapIO::AI, element_as_indicator->GetIoAdr());
            CHECK_EQUAL(42, element_as_indicator->GetHighScale());
            CHECK_EQUAL(19, element_as_indicator->GetLowScale());
            CHECK_EQUAL(2, element_as_indicator->GetDecimalPoint());
            matched++;
        }
    }
    delete testable.GetSelectedElement();
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, indicator_element_has_default_param_V1) {
    int matched = 0;
    Wire stored_element;
    ElementsBox testable(115, &stored_element, false);
    for (auto *element : testable) {
        auto *element_as_indicator = Indicator::TryToCast(element);
        if (element_as_indicator != NULL) {
            CHECK_EQUAL(MapIO::V1, element_as_indicator->GetIoAdr());
            matched++;
        }
    }
    delete testable.GetSelectedElement();
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, copy_params_for_wifi_binding_element) {
    int matched = 0;
    WiFiBinding stored_element(MapIO::AI, "wifi_test");
    ElementsBox testable(100, &stored_element, false);
    for (auto *element : testable) {
        auto *element_as_wifi_binding = WiFiBinding::TryToCast(element);
        if (element_as_wifi_binding != NULL) {
            CHECK_EQUAL(MapIO::AI, element_as_wifi_binding->GetIoAdr());
            STRCMP_EQUAL("wifi_test", element_as_wifi_binding->GetSsid());
            matched++;
        }
    }
    delete testable.GetSelectedElement();
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, wifi_binding_element_has_default_param_V1) {
    int matched = 0;
    Wire stored_element;
    ElementsBox testable(115, &stored_element, false);
    for (auto *element : testable) {
        auto *element_as_wifi_binding = WiFiBinding::TryToCast(element);
        if (element_as_wifi_binding != NULL) {
            CHECK_EQUAL(MapIO::V1, element_as_wifi_binding->GetIoAdr());
            matched++;
        }
    }
    delete testable.GetSelectedElement();
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, no_available_space_for_timers_and_comparators) {
    InputNC stored_element(MapIO::V1);
    ElementsBox testable(7, &stored_element, false);
    CHECK_EQUAL(6, testable.size());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, use_GetElementType_from_selected) {
    InputNC stored_element(MapIO::V1);
    ElementsBox testable(100, &stored_element, false);
    CHECK_EQUAL(TvElementType::et_InputNC, testable.GetElementType());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, use_DoAction_from_selected) {
    mock().enable();
    volatile uint16_t adc = 42 / 0.1;
    mock().expectNCalls(1, "vTaskDelay").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock()
        .expectNCalls(1, "adc_read")
        .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));
    Controller::GetIOValues().AI.value = LogicElement::MinValue;
    Controller::GetIOValues().AI.required = true;
    CHECK_TRUE(Controller::SampleIOValues());

    TestableComparatorEq fake_doaction_element(42 / 0.4, MapIO::AI);
    ElementsBox testable(100, &fake_doaction_element, false);
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, testable.GetSelectedElement()->GetState());

    CHECK_FALSE(fake_doaction_element.DoAction_called);
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, Render_calls_a_function_on_the_inner_element) {
    TestableComparatorEq fake_rendering_element(42, MapIO::AI);
    ElementsBox testable(100, &fake_rendering_element, false);
    Point start_point = { 0, INCOME_RAIL_TOP };
    testable.Render(frame_buffer, LogicItemState::lisActive, &start_point);
    delete testable.GetSelectedElement();

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(32, start_point.x);
}

TEST(LogicElementsBoxTestsGroup, SelectNext__change__selected_index__to_backward) {
    ComparatorEq stored_element(42, MapIO::AI);
    ElementsBox testable(100, &stored_element, false);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_TimerSecs, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_InputNO, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_InputNC, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_Wire, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_DecOutput, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_IncOutput, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_ResetOutput, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_SetOutput, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_DirectOutput, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_WiFiBinding, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_Indicator, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_ComparatorLs, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_ComparatorLE, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_ComparatorGr, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_ComparatorGE, testable.GetElementType());
    testable.SelectNext();
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, SelectPrior_selecting_elements_in_loop) {
    ComparatorEq stored_element(42, MapIO::AI);
    ElementsBox testable(100, &stored_element, false);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_ComparatorGE, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_ComparatorGr, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_ComparatorLE, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_ComparatorLs, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_Indicator, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_WiFiBinding, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_DirectOutput, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_SetOutput, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_ResetOutput, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_IncOutput, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_DecOutput, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_Wire, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_InputNC, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_InputNO, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_TimerSecs, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable.GetElementType());
    testable.SelectPrior();

    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, HandleButtonSelect_first_call_switch_element_to_editing) {
    ComparatorEq stored_element(42, MapIO::AI);
    stored_element.Select();
    ElementsBox testable(100, &stored_element, false);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.Change();
    CHECK_TRUE(stored_element.Editing());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, No_memleak_if_selection_changes) {
    ComparatorEq stored_element(42, MapIO::AI);
    ElementsBox testable(100, &stored_element, false);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_ComparatorGE, testable.GetElementType());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, No_memleak_if_no_selection_changes) {
    ComparatorEq stored_element(42, MapIO::AI);
    ElementsBox testable(100, &stored_element, false);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, In_editing_no_memleak_if_selection_changes) {
    ComparatorEq stored_element(42, MapIO::AI);
    ElementsBox testable(100, &stored_element, false);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.BeginEditing();
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_ComparatorGE, testable.GetElementType());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, In_editing_no_memleak_if_no_selection_changes) {
    ComparatorEq stored_element(42, MapIO::AI);
    ElementsBox testable(100, &stored_element, false);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.BeginEditing();
    delete testable.GetSelectedElement();
}