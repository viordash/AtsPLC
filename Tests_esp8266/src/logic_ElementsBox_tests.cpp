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

static FrameBuffer frame_buffer = {};
static WiFiService *wifi_service;
static DatetimeService *datetime_service;

static ProcessWakeupService *process_wakeup_service;

TEST_GROUP(LogicElementsBoxTestsGroup){ //
                                        TEST_SETUP(){ mock().disable();
memset(&frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));
wifi_service = new WiFiService();
datetime_service = new DatetimeService();
process_wakeup_service = new ProcessWakeupService();
Controller::Start(NULL, wifi_service, NULL, datetime_service, process_wakeup_service);
}

TEST_TEARDOWN() {
    Controller::V1.Unbind();
    Controller::V2.Unbind();
    Controller::V3.Unbind();
    Controller::V4.Unbind();
    Controller::Stop();
    delete process_wakeup_service;
    delete wifi_service;
    delete datetime_service;
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

        void Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) override {
            (void)fb;
            (void)prev_elem_state;
            MonitorLogicElement::Render(start_point);
        }
    };

    class TestableElementsBox : public ElementsBox {
      public:
        TestableElementsBox(uint8_t fill_wire, LogicElement *source_element, Options options)
            : ElementsBox(fill_wire, source_element, options) {
        }
        LogicElement *PublicMorozov_at(size_t index) {
            return items[index];
        }
        std::vector<LogicElement *> &PublicMorozov_Get_items() {
            return items;
        }
    };
} // namespace

TEST(LogicElementsBoxTestsGroup, box_fill_elements) {
    InputNC stored_element(MapIO::V1);
    TestableElementsBox testable(DISPLAY_WIDTH - INCOME_RAIL_WIDTH - SCROLLBAR_WIDTH,
                                 &stored_element,
                                 ElementsBox::Options::show_output_elements);
    CHECK_EQUAL(23, testable.GetCount());
    CHECK_EQUAL(TvElementType::et_InputNC, testable.PublicMorozov_at(0)->GetElementType());
    CHECK_EQUAL(TvElementType::et_InputNO, testable.PublicMorozov_at(1)->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerSecs, testable.PublicMorozov_at(2)->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable.PublicMorozov_at(3)->GetElementType());
    CHECK_EQUAL(TvElementType::et_SquareWaveGenerator,
                testable.PublicMorozov_at(4)->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.PublicMorozov_at(5)->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorNe, testable.PublicMorozov_at(6)->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorGE, testable.PublicMorozov_at(7)->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorGr, testable.PublicMorozov_at(8)->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorLE, testable.PublicMorozov_at(9)->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorLs, testable.PublicMorozov_at(10)->GetElementType());
    CHECK_EQUAL(TvElementType::et_Indicator, testable.PublicMorozov_at(11)->GetElementType());
    CHECK_EQUAL(TvElementType::et_WiFiBinding, testable.PublicMorozov_at(12)->GetElementType());
    CHECK_EQUAL(TvElementType::et_WiFiStaBinding, testable.PublicMorozov_at(13)->GetElementType());
    CHECK_EQUAL(TvElementType::et_WiFiApBinding, testable.PublicMorozov_at(14)->GetElementType());
    CHECK_EQUAL(TvElementType::et_DateTimeBinding, testable.PublicMorozov_at(15)->GetElementType());
    CHECK_EQUAL(TvElementType::et_Settings, testable.PublicMorozov_at(16)->GetElementType());
    CHECK_EQUAL(TvElementType::et_DirectOutput, testable.PublicMorozov_at(17)->GetElementType());
    CHECK_EQUAL(TvElementType::et_SetOutput, testable.PublicMorozov_at(18)->GetElementType());
    CHECK_EQUAL(TvElementType::et_ResetOutput, testable.PublicMorozov_at(19)->GetElementType());
    CHECK_EQUAL(TvElementType::et_IncOutput, testable.PublicMorozov_at(20)->GetElementType());
    CHECK_EQUAL(TvElementType::et_DecOutput, testable.PublicMorozov_at(21)->GetElementType());
    CHECK_EQUAL(TvElementType::et_Wire, testable.PublicMorozov_at(22)->GetElementType());
}

TEST(LogicElementsBoxTestsGroup, hide_output_elements) {
    Wire stored_element;
    TestableElementsBox testable(DISPLAY_WIDTH - INCOME_RAIL_WIDTH - SCROLLBAR_WIDTH,
                                 &stored_element,
                                 {});
    CHECK_EQUAL(18, testable.GetCount());
    CHECK_EQUAL(TvElementType::et_InputNC, testable.PublicMorozov_at(0)->GetElementType());
    CHECK_EQUAL(TvElementType::et_InputNO, testable.PublicMorozov_at(1)->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerSecs, testable.PublicMorozov_at(2)->GetElementType());
    CHECK_EQUAL(TvElementType::et_TimerMSecs, testable.PublicMorozov_at(3)->GetElementType());
    CHECK_EQUAL(TvElementType::et_SquareWaveGenerator,
                testable.PublicMorozov_at(4)->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.PublicMorozov_at(5)->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorNe, testable.PublicMorozov_at(6)->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorGE, testable.PublicMorozov_at(7)->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorGr, testable.PublicMorozov_at(8)->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorLE, testable.PublicMorozov_at(9)->GetElementType());
    CHECK_EQUAL(TvElementType::et_ComparatorLs, testable.PublicMorozov_at(10)->GetElementType());
    CHECK_EQUAL(TvElementType::et_Indicator, testable.PublicMorozov_at(11)->GetElementType());
    CHECK_EQUAL(TvElementType::et_WiFiBinding, testable.PublicMorozov_at(12)->GetElementType());
    CHECK_EQUAL(TvElementType::et_WiFiStaBinding, testable.PublicMorozov_at(13)->GetElementType());
    CHECK_EQUAL(TvElementType::et_WiFiApBinding, testable.PublicMorozov_at(14)->GetElementType());
    CHECK_EQUAL(TvElementType::et_DateTimeBinding, testable.PublicMorozov_at(15)->GetElementType());
    CHECK_EQUAL(TvElementType::et_Settings, testable.PublicMorozov_at(16)->GetElementType());
    CHECK_EQUAL(TvElementType::et_Wire, testable.PublicMorozov_at(17)->GetElementType());
}

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_input_element) {
    int matched = 0;
    InputNC stored_element(MapIO::V2);
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    for (auto *element : testable.PublicMorozov_Get_items()) {
        auto *element_as_commonInput = CommonInput::TryToCast(element);
        if (element_as_commonInput != NULL) {
            CHECK_EQUAL(MapIO::V2, element_as_commonInput->GetIoAdr());
            matched++;
        }
    }
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_comparator_element) {
    int matched = 0;
    ComparatorEq stored_element(42, MapIO::AI);
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    for (auto *element : testable.PublicMorozov_Get_items()) {
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
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup,
     takes_params_from_stored_element__set_reference_to_zero_by_default_for_comparators) {
    int matched = 0;
    InputNC stored_element(MapIO::V1);
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    for (auto *element : testable.PublicMorozov_Get_items()) {
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
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_TimerSec_element) {
    int matched = 0;
    TimerSecs stored_element(42);
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    for (auto *element : testable.PublicMorozov_Get_items()) {
        auto *element_as_commonTimer = CommonTimer::TryToCast(element);
        if (element_as_commonTimer != NULL) {
            auto *element_as_TimerMSecs = TimerMSecs::TryToCast(element_as_commonTimer);
            if (element_as_TimerMSecs != NULL) {
                CHECK_EQUAL(42 * 1000000LL, element_as_TimerMSecs->GetTimeUs());
                matched++;
            }
        }
    }
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_TimerMSec_element) {
    int matched = 0;
    TimerMSecs stored_element(42000);
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    for (auto *element : testable.PublicMorozov_Get_items()) {
        auto *element_as_commonTimer = CommonTimer::TryToCast(element);
        if (element_as_commonTimer != NULL) {
            auto *element_as_TimerSecs = TimerSecs::TryToCast(element_as_commonTimer);
            if (element_as_TimerSecs != NULL) {
                CHECK_EQUAL(42 * 1000000LL, element_as_TimerSecs->GetTimeUs());
                matched++;
            }
        }
    }
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup,
     takes_params_from_stored_element__set_default_delaytime_for_timers) {
    int matched = 0;
    InputNC stored_element(MapIO::V1);
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    for (auto *element : testable.PublicMorozov_Get_items()) {
        auto *element_as_commonTimer = CommonTimer::TryToCast(element);
        if (element_as_commonTimer != NULL) {
            auto *element_as_TimerSecs = TimerSecs::TryToCast(element_as_commonTimer);
            if (element_as_TimerSecs != NULL) {
                CHECK_EQUAL(1000000LL, element_as_TimerSecs->GetTimeUs());
                matched++;
            }

            auto *element_as_TimerMSecs = TimerMSecs::TryToCast(element_as_commonTimer);
            if (element_as_TimerMSecs != NULL) {
                CHECK_EQUAL(50000LL, element_as_TimerMSecs->GetTimeUs());
                matched++;
            }
        }
    }
    CHECK_COMPARE(matched, >, 1);
}

TEST(LogicElementsBoxTestsGroup, takes_params_from_stored_output_element) {
    int matched = 0;
    IncOutput stored_element(MapIO::O1);
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    for (auto *element : testable.PublicMorozov_Get_items()) {
        auto *element_as_commonOutput = CommonOutput::TryToCast(element);
        if (element_as_commonOutput != NULL) {
            CHECK_EQUAL(MapIO::O1, element_as_commonOutput->GetIoAdr());
            matched++;
        }
    }
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, takes_params_for_wire) {
    int matched = 0;
    InputNC stored_element(MapIO::V1);
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    for (auto *element : testable.PublicMorozov_Get_items()) {
        auto *element_as_wire = Wire::TryToCast(element);
        if (element_as_wire != NULL) {
            CHECK_EQUAL(24, element_as_wire->GetWidth());
            matched++;
        }
    }
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, copy_params_for_indicator_element) {
    int matched = 0;
    Indicator stored_element(MapIO::AI);
    stored_element.SetHighScale(42);
    stored_element.SetLowScale(19);
    stored_element.SetDecimalPoint(2);
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    for (auto *element : testable.PublicMorozov_Get_items()) {
        auto *element_as_indicator = Indicator::TryToCast(element);
        if (element_as_indicator != NULL) {
            CHECK_EQUAL(MapIO::AI, element_as_indicator->GetIoAdr());
            CHECK_EQUAL(42, element_as_indicator->GetHighScale());
            CHECK_EQUAL(19, element_as_indicator->GetLowScale());
            CHECK_EQUAL(2, element_as_indicator->GetDecimalPoint());
            matched++;
        }
    }
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, indicator_element_has_default_param_V1) {
    int matched = 0;
    Wire stored_element;
    TestableElementsBox testable(115, &stored_element, ElementsBox::Options::show_output_elements);
    for (auto *element : testable.PublicMorozov_Get_items()) {
        auto *element_as_indicator = Indicator::TryToCast(element);
        if (element_as_indicator != NULL) {
            CHECK_EQUAL(MapIO::V1, element_as_indicator->GetIoAdr());
            matched++;
        }
    }
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, copy_params_for_wifi_binding_element) {
    int matched = 0;
    WiFiBinding stored_element(MapIO::AI, "wifi_test");
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    for (auto *element : testable.PublicMorozov_Get_items()) {
        auto *element_as_wifi_binding = WiFiBinding::TryToCast(element);
        if (element_as_wifi_binding != NULL) {
            CHECK_EQUAL(MapIO::AI, element_as_wifi_binding->GetIoAdr());
            STRCMP_EQUAL("wifi_test", element_as_wifi_binding->GetSsid());
            matched++;
        }
    }
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, wifi_binding_element_has_default_param_V1) {
    int matched = 0;
    Wire stored_element;
    TestableElementsBox testable(115, &stored_element, ElementsBox::Options::show_output_elements);
    for (auto *element : testable.PublicMorozov_Get_items()) {
        auto *element_as_wifi_binding = WiFiBinding::TryToCast(element);
        if (element_as_wifi_binding != NULL) {
            CHECK_EQUAL(MapIO::V1, element_as_wifi_binding->GetIoAdr());
            matched++;
        }
    }
    CHECK_COMPARE(matched, >, 0);
}

TEST(LogicElementsBoxTestsGroup, no_available_space_for_timers_and_comparators) {
    InputNC stored_element(MapIO::V1);
    TestableElementsBox testable(5, &stored_element, ElementsBox::Options::show_output_elements);
    CHECK_EQUAL(6, testable.GetCount());
}

TEST(LogicElementsBoxTestsGroup, use_GetElementType_from_selected) {
    InputNC stored_element(MapIO::V1);
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    CHECK_EQUAL(TvElementType::et_InputNC, testable.GetElementType());
}

TEST(LogicElementsBoxTestsGroup, use_DoAction_from_selected) {
    mock().enable();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock().expectNCalls(1, "vTaskDelay").ignoreOtherParameters();

    volatile uint16_t adc = 42 / 0.1;
    mock()
        .expectNCalls(1, "adc_read")
        .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));

    Controller::FetchIOValues();

    TestableComparatorEq fake_doaction_element(42 / 0.4, MapIO::AI);
    fake_doaction_element.DoAction_result = true;
    TestableElementsBox testable(100, &fake_doaction_element, {});
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));

    CHECK_TRUE(fake_doaction_element.DoAction_called);
}

TEST(LogicElementsBoxTestsGroup, Render_calls_a_function_on_the_inner_element) {
    TestableComparatorEq fake_rendering_element(42, MapIO::AI);
    TestableElementsBox testable(100, &fake_rendering_element, {});
    Point start_point = { 0, INCOME_RAIL_TOP };
    testable.Render(&frame_buffer, LogicItemState::lisActive, &start_point);

    CHECK_TRUE(fake_rendering_element.Render_called);
}

TEST(LogicElementsBoxTestsGroup, SelectNext__change__selected_index__to_backward) {
    ComparatorEq stored_element(42, MapIO::AI);
    TestableElementsBox testable(
        100,
        &stored_element,
        (ElementsBox::Options)(ElementsBox::Options::show_output_elements
                               | ElementsBox::Options::show_continuation_in
                               | ElementsBox::Options::show_continuation_out));
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_SquareWaveGenerator, testable.GetElementType());
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
    CHECK_EQUAL(TvElementType::et_ContinuationOut, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_ContinuationIn, testable.GetElementType());
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
    CHECK_EQUAL(TvElementType::et_Settings, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_DateTimeBinding, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_WiFiApBinding, testable.GetElementType());
    testable.SelectNext();
    CHECK_EQUAL(TvElementType::et_WiFiStaBinding, testable.GetElementType());
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
    CHECK_EQUAL(TvElementType::et_ComparatorNe, testable.GetElementType());
    testable.SelectNext();
}

TEST(LogicElementsBoxTestsGroup, SelectPrior_selecting_elements_in_loop) {
    ComparatorEq stored_element(42, MapIO::AI);
    TestableElementsBox testable(
        100,
        &stored_element,
        (ElementsBox::Options)(ElementsBox::Options::show_output_elements
                               | ElementsBox::Options::show_continuation_in
                               | ElementsBox::Options::show_continuation_out));
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_ComparatorNe, testable.GetElementType());
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
    CHECK_EQUAL(TvElementType::et_WiFiStaBinding, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_WiFiApBinding, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_DateTimeBinding, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_Settings, testable.GetElementType());
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
    CHECK_EQUAL(TvElementType::et_ContinuationIn, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_ContinuationOut, testable.GetElementType());
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
    CHECK_EQUAL(TvElementType::et_SquareWaveGenerator, testable.GetElementType());
    testable.SelectPrior();
}

TEST(LogicElementsBoxTestsGroup, HandleButtonSelect_first_call_switch_element_to_editing) {
    ComparatorEq stored_element(42, MapIO::AI);
    stored_element.Select();
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.Change();
    CHECK_TRUE(stored_element.Editing());
}

TEST(LogicElementsBoxTestsGroup, No_memleak_if_selection_changes) {
    auto stored_element = new ComparatorEq(42, MapIO::AI);
    TestableElementsBox testable(100, stored_element, {});
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_ComparatorNe, testable.GetElementType());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, No_memleak_if_no_selection_changes) {
    auto stored_element = new ComparatorEq(42, MapIO::AI);
    TestableElementsBox testable(100, stored_element, {});
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    delete stored_element;
}

TEST(LogicElementsBoxTestsGroup, In_editing_no_memleak_if_selection_changes) {
    auto stored_element = new ComparatorEq(42, MapIO::AI);
    TestableElementsBox testable(100, stored_element, {});
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.BeginEditing();
    testable.SelectPrior();
    CHECK_EQUAL(TvElementType::et_ComparatorNe, testable.GetElementType());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, No_memleak_if_stored_element_is_not_offered) {
    auto stored_element = new DirectOutput(MapIO::O1);
    TestableElementsBox testable(100, stored_element, {});
    CHECK_EQUAL(TvElementType::et_InputNC, testable.GetElementType());
    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, In_editing_no_memleak_if_no_selection_changes) {
    ComparatorEq stored_element(42, MapIO::AI);
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    CHECK_EQUAL(TvElementType::et_ComparatorEq, testable.GetElementType());
    testable.BeginEditing();
}

TEST(LogicElementsBoxTestsGroup, SelectNext_on_WiFiApBinding_calls_DetachElement) {
    auto stored_element = new WiFiApBinding();
    stored_element->SetIoAdr(MapIO::V1);
    stored_element->SetSsid("ssid");
    stored_element->SetPassword("secret");
    stored_element->SetMac("0123456789AB");
    stored_element->DoAction(true, LogicItemState::lisActive);
    CHECK_EQUAL(LogicItemState::lisActive, stored_element->GetState());
    CHECK_TRUE(Controller::V1.BindedToWiFi());

    TestableElementsBox testable(100, stored_element, {});
    CHECK_EQUAL(TvElementType::et_WiFiApBinding, testable.GetElementType());

    testable.SelectNext();

    CHECK_EQUAL(LogicItemState::lisPassive, stored_element->GetState());
    CHECK_FALSE(Controller::V1.BindedToWiFi());

    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, SelectNext_on_WiFiBinding_calls_DetachElement) {
    auto stored_element = new WiFiBinding();
    stored_element->SetIoAdr(MapIO::V1);
    stored_element->DoAction(true, LogicItemState::lisActive);
    CHECK_EQUAL(LogicItemState::lisActive, stored_element->GetState());
    CHECK_TRUE(Controller::V1.BindedToWiFi());

    TestableElementsBox testable(100, stored_element, {});
    CHECK_EQUAL(TvElementType::et_WiFiBinding, testable.GetElementType());

    testable.SelectNext();

    CHECK_EQUAL(LogicItemState::lisPassive, stored_element->GetState());
    CHECK_FALSE(Controller::V1.BindedToWiFi());

    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, SelectNext_on_WiFiStaBinding_calls_DetachElement) {
    auto stored_element = new WiFiStaBinding();
    stored_element->SetIoAdr(MapIO::V1);
    stored_element->DoAction(true, LogicItemState::lisActive);
    CHECK_EQUAL(LogicItemState::lisActive, stored_element->GetState());
    CHECK_TRUE(Controller::V1.BindedToWiFi());

    TestableElementsBox testable(100, stored_element, {});
    CHECK_EQUAL(TvElementType::et_WiFiStaBinding, testable.GetElementType());

    testable.SelectNext();

    CHECK_EQUAL(LogicItemState::lisPassive, stored_element->GetState());
    CHECK_FALSE(Controller::V1.BindedToWiFi());

    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, SelectNext_on_DateTimeBinding_calls_DetachElement) {
    auto stored_element = new DateTimeBinding();
    stored_element->SetIoAdr(MapIO::V1);
    stored_element->DoAction(true, LogicItemState::lisActive);
    CHECK_EQUAL(LogicItemState::lisActive, stored_element->GetState());
    CHECK_TRUE(Controller::V1.BindedToDateTime());

    TestableElementsBox testable(100, stored_element, {});
    CHECK_EQUAL(TvElementType::et_DateTimeBinding, testable.GetElementType());

    testable.SelectNext();

    CHECK_EQUAL(LogicItemState::lisPassive, stored_element->GetState());
    CHECK_FALSE(Controller::V1.BindedToDateTime());

    delete testable.GetSelectedElement();
}

TEST(LogicElementsBoxTestsGroup, output_elements_will_not_accept_address_which_is_only_for_inputs) {
    int matched = 0;
    Indicator stored_element(MapIO::AI);
    TestableElementsBox testable(100, &stored_element, ElementsBox::Options::show_output_elements);
    for (auto *element : testable.PublicMorozov_Get_items()) {
        auto *element_as_commonOutput = CommonOutput::TryToCast(element);
        if (element_as_commonOutput != NULL) {
            CHECK_EQUAL(MapIO::V1, element_as_commonOutput->GetIoAdr());
            matched++;
        }
    }
    CHECK_EQUAL(5, matched);
}