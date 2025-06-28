
#include "LogicProgram/ElementsBox.h"
#include "Display/display.h"
#include "LogicProgram/LogicProgram.h"
#include "LogicProgram/Serializer/LogicElementFactory.h"
#include "esp_err.h"
#include "esp_log.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ElementsBox = "ElementsBox";

ElementsBox::ElementsBox(uint8_t fill_wire, LogicElement *source_element, Options options)
    : LogicElement() {
    source_element_width = 0;
    source_element->BeginEditing();
    selected_index = 0;
    force_do_action_result = false;
    state = source_element->state;
    CalcEntirePlaceWidth(source_element);
    place_width = source_element_width + fill_wire;
    Fill(source_element, options);
}

ElementsBox::~ElementsBox() {
    auto selected_it = std::find(begin(), end(), GetSelectedElement());
    erase(selected_it);

    while (!empty()) {
        auto it = begin();
        auto element = *it;
        erase(it);
        ESP_LOGD(TAG_ElementsBox, "delete elem: %p", element);
        delete element;
    }
}

void ElementsBox::DetachElement(LogicElement *element) {
    ESP_LOGD(TAG_ElementsBox, "attempt of detach element with type:%u", element->GetElementType());
    auto *wifi_binding = WiFiBinding::TryToCast(element);
    if (wifi_binding != NULL) {
        if (wifi_binding->DoAction(true, LogicItemState::lisPassive)) {
            ESP_LOGI(TAG_ElementsBox,
                     "detach WiFiBinding, '%s', ssid:%s",
                     wifi_binding->GetLabel(),
                     wifi_binding->GetSsid());
        }
        return;
    }

    auto *wifi_ap_binding = WiFiApBinding::TryToCast(element);
    if (wifi_ap_binding != NULL) {
        if (wifi_ap_binding->DoAction(true, LogicItemState::lisPassive)) {
            ESP_LOGI(TAG_ElementsBox,
                     "detach WiFiApBinding, '%s', ssid:%s",
                     wifi_ap_binding->GetLabel(),
                     wifi_ap_binding->GetSsid());
        }
        return;
    }

    auto *wifi_sta_binding = WiFiStaBinding::TryToCast(element);
    if (wifi_sta_binding != NULL) {
        if (wifi_sta_binding->DoAction(true, LogicItemState::lisPassive)) {
            ESP_LOGI(TAG_ElementsBox, "detach WiFiStaBinding, '%s'", wifi_sta_binding->GetLabel());
        }
        return;
    }
}

void ElementsBox::CalcEntirePlaceWidth(LogicElement *source_element) {
    uint8_t *frame_buffer = new uint8_t[DISPLAY_HEIGHT_IN_BYTES * DISPLAY_WIDTH];
    uint8_t start_point_x = IsOutputElement(source_element->GetElementType()) //
                              ? DISPLAY_WIDTH / 2
                              : INCOME_RAIL_WIDTH;
    Point start_point = { start_point_x, DISPLAY_HEIGHT / 2 };
    if (source_element->Render(frame_buffer, LogicItemState::lisPassive, &start_point)) {
        if (IsOutputElement(source_element->GetElementType())) {
            source_element_width = start_point_x - start_point.x;
        } else {
            source_element_width = start_point.x - start_point_x;
        }
    }
    delete[] frame_buffer;
}

void ElementsBox::CopyParamsToInputElement(LogicElement *source_element, InputElement *input) {
    if (input == NULL) {
        return;
    }
    MapIO io_adr = MapIO::V1;

    auto *source_element_as_input = InputElement::TryToCast(source_element);
    if (source_element_as_input != NULL) {
        io_adr = source_element_as_input->GetIoAdr();
    }

    input->SetIoAdr(io_adr);
}

bool ElementsBox::CopyParamsToCommonOutputElement(LogicElement *source_element,
                                                  CommonOutput *common_output) {
    if (common_output == NULL) {
        return false;
    }
    MapIO io_adr = MapIO::V1;

    auto *source_element_as_output = CommonOutput::TryToCast(source_element);
    if (source_element_as_output != NULL) {
        io_adr = source_element_as_output->GetIoAdr();
    }

    common_output->SetIoAdr(io_adr);
    return true;
}

bool ElementsBox::CopyParamsToCommonComparator(LogicElement *source_element,
                                               CommonComparator *common_comparator) {
    if (common_comparator == NULL) {
        return false;
    }

    auto *new_element_as_commonComparator = CommonComparator::TryToCast(common_comparator);
    if (new_element_as_commonComparator != NULL) {
        uint8_t ref_percent04 = 0;

        auto *source_element_as_commonInput = CommonInput::TryToCast(source_element);
        if (source_element_as_commonInput != NULL) {
            auto *source_element_as_commonComparator =
                CommonComparator::TryToCast(source_element_as_commonInput);

            if (source_element_as_commonComparator != NULL) {
                ref_percent04 = source_element_as_commonComparator->GetReference();
            }
        }
        new_element_as_commonComparator->SetReference(ref_percent04);
    }
    return true;
}

bool ElementsBox::CopyParamsToCommonTimer(LogicElement *source_element, CommonTimer *common_timer) {
    if (common_timer == NULL) {
        return false;
    }

    uint64_t time_us = 0;
    auto *source_element_as_commonTimer = CommonTimer::TryToCast(source_element);
    if (source_element_as_commonTimer != NULL) {
        auto *source_element_as_timerSecs = TimerSecs::TryToCast(source_element_as_commonTimer);
        if (source_element_as_timerSecs != NULL) {
            time_us = source_element_as_timerSecs->GetTimeUs();
        } else {
            auto *source_element_as_timerMSecs =
                TimerMSecs::TryToCast(source_element_as_commonTimer);
            if (source_element_as_timerMSecs != NULL) {
                time_us = source_element_as_timerMSecs->GetTimeUs();
            }
        }
    }

    auto *new_element_as_TimerSecs = TimerSecs::TryToCast(common_timer);
    if (new_element_as_TimerSecs != NULL) {
        new_element_as_TimerSecs->SetTime(time_us / 1000000LL);
    } else {
        auto *new_element_as_TimerMSecs = TimerMSecs::TryToCast(common_timer);
        if (new_element_as_TimerMSecs != NULL) {
            new_element_as_TimerMSecs->SetTime(time_us / 1000LL);
        }
    }
    return true;
}

bool ElementsBox::CopyParamsToIndicator(LogicElement *source_element, Indicator *indicator) {
    if (indicator == NULL) {
        return false;
    }

    auto *source_element_as_indicator = Indicator::TryToCast(source_element);
    if (source_element_as_indicator != NULL) {
        indicator->SetLowScale(source_element_as_indicator->GetLowScale());
        indicator->SetHighScale(source_element_as_indicator->GetHighScale());
        indicator->SetDecimalPoint(source_element_as_indicator->GetDecimalPoint());
        return true;
    }

    return true;
}

bool ElementsBox::CopyParamsToWiFiBinding(LogicElement *source_element, WiFiBinding *binding) {
    if (binding == NULL) {
        return false;
    }

    auto *source_element_as_wifi_binding = WiFiBinding::TryToCast(source_element);
    if (source_element_as_wifi_binding != NULL) {
        binding->SetSsid(source_element_as_wifi_binding->GetSsid());
        return true;
    }
    binding->SetSsid("AtsPLC");
    return true;
}

bool ElementsBox::CopyParamsToWiFiApBinding(LogicElement *source_element, WiFiApBinding *binding) {
    if (binding == NULL) {
        return false;
    }

    auto *source_element_as_wifi_binding = WiFiBinding::TryToCast(source_element);
    if (source_element_as_wifi_binding != NULL) {
        binding->SetSsid(source_element_as_wifi_binding->GetSsid());
        binding->SetPassword("ats-PLC0");
        binding->SetMac("************");
        return true;
    }

    auto *source_element_as_wifi_ap_binding = WiFiApBinding::TryToCast(source_element);
    if (source_element_as_wifi_ap_binding != NULL) {
        binding->SetSsid(source_element_as_wifi_ap_binding->GetSsid());
        binding->SetPassword(source_element_as_wifi_ap_binding->GetPassword());
        binding->SetMac(source_element_as_wifi_ap_binding->GetMac());
        return true;
    }
    binding->SetSsid("AtsPLC");
    binding->SetPassword("ats-PLC0");
    binding->SetMac("************");
    return true;
}

void ElementsBox::TakeParamsFromStoredElement(LogicElement *source_element,
                                              LogicElement *new_element) {

    CopyParamsToInputElement(source_element, InputElement::TryToCast(new_element));

    if (CopyParamsToCommonOutputElement(source_element, CommonOutput::TryToCast(new_element))) {
        return;
    }
    if (CopyParamsToCommonComparator(source_element, CommonComparator::TryToCast(new_element))) {
        return;
    }
    if (CopyParamsToCommonTimer(source_element, CommonTimer::TryToCast(new_element))) {
        return;
    }
    if (CopyParamsToIndicator(source_element, Indicator::TryToCast(new_element))) {
        return;
    }
    if (CopyParamsToWiFiBinding(source_element, WiFiBinding::TryToCast(new_element))) {
        return;
    }
    if (CopyParamsToWiFiApBinding(source_element, WiFiApBinding::TryToCast(new_element))) {
        return;
    }

    auto as_wire = Wire::TryToCast(new_element);
    if (as_wire != NULL) {
        as_wire->SetWidth(source_element_width);
        return;
    }
}

void ElementsBox::AppendStandartElement(LogicElement *source_element,
                                        TvElementType element_type,
                                        uint8_t *frame_buffer) {
    if (source_element->GetElementType() == element_type) {
        selected_index = size();
        push_back(source_element);
        return;
    }

    auto new_element = LogicElementFactory::Create(element_type);
    if (new_element == NULL) {
        return;
    }
    TakeParamsFromStoredElement(source_element, new_element);

    uint8_t start_point_x = IsOutputElement(element_type) //
                              ? DISPLAY_WIDTH / 2
                              : INCOME_RAIL_WIDTH;
    Point start_point = { start_point_x, DISPLAY_HEIGHT / 2 };
    if (!new_element->Render(frame_buffer, LogicItemState::lisPassive, &start_point)) {
        delete new_element;
        return;
    }
    uint8_t new_element_width = 0;
    if (IsOutputElement(element_type)) {
        new_element_width = start_point_x - start_point.x;
    } else {
        new_element_width = start_point.x - start_point_x;
    }
    bool element_not_fit = new_element_width > place_width;
    ESP_LOGD(TAG_ElementsBox, "new_element_width: %u", new_element_width);
    if (element_not_fit) {
        delete new_element;
        return;
    }
    new_element->BeginEditing();
    push_back(new_element);
}

void ElementsBox::Fill(LogicElement *source_element, Options options) {
    uint8_t *frame_buffer = new uint8_t[DISPLAY_HEIGHT_IN_BYTES * DISPLAY_WIDTH];

    AppendStandartElement(source_element, TvElementType::et_InputNC, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_InputNO, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_TimerSecs, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_TimerMSecs, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_SquareWaveGenerator, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_ComparatorEq, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_ComparatorNe, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_ComparatorGE, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_ComparatorGr, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_ComparatorLE, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_ComparatorLs, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_Indicator, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_WiFiBinding, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_WiFiStaBinding, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_WiFiApBinding, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_DateTimeBinding, frame_buffer);

    if (options & Options::show_output_elements) {
        AppendStandartElement(source_element, TvElementType::et_DirectOutput, frame_buffer);
        AppendStandartElement(source_element, TvElementType::et_SetOutput, frame_buffer);
        AppendStandartElement(source_element, TvElementType::et_ResetOutput, frame_buffer);
        AppendStandartElement(source_element, TvElementType::et_IncOutput, frame_buffer);
        AppendStandartElement(source_element, TvElementType::et_DecOutput, frame_buffer);
    }
    AppendStandartElement(source_element, TvElementType::et_Settings, frame_buffer);
    AppendStandartElement(source_element, TvElementType::et_Wire, frame_buffer);

    delete[] frame_buffer;
}

LogicElement *ElementsBox::GetSelectedElement() {
    return (*this)[selected_index];
}

bool ElementsBox::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    bool res =
        GetSelectedElement()->DoAction(prev_elem_changed || force_do_action_result, prev_elem_state)
        || force_do_action_result;
    state = GetSelectedElement()->state;
    force_do_action_result = false;
    return res;
}

bool ElementsBox::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = GetSelectedElement()->Render(fb, prev_elem_state, start_point);
    return res;
}

size_t ElementsBox::Serialize(uint8_t *buffer, size_t buffer_size) {
    return GetSelectedElement()->Serialize(buffer, buffer_size);
}

size_t ElementsBox::Deserialize(uint8_t *buffer, size_t buffer_size) {
    return GetSelectedElement()->Deserialize(buffer, buffer_size);
}

TvElementType ElementsBox::GetElementType() {
    return GetSelectedElement()->GetElementType();
}

void ElementsBox::SelectPrior() {
    bool selected_in_editing_property =
        GetSelectedElement()->Editing() && GetSelectedElement()->InEditingProperty();

    ESP_LOGI(TAG_ElementsBox,
             "SelectPrior, selected_index:%d, in_editing:%d",
             selected_index,
             selected_in_editing_property);

    if (selected_in_editing_property) {
        GetSelectedElement()->SelectPrior();
        return;
    }

    DetachElement((*this)[selected_index]);
    selected_index++;
    if (selected_index >= (int)size()) {
        selected_index = 0;
    }
    force_do_action_result = true;
}

void ElementsBox::SelectNext() {
    bool selected_in_editing_property =
        GetSelectedElement()->Editing() && GetSelectedElement()->InEditingProperty();

    ESP_LOGI(TAG_ElementsBox,
             "SelectNext, selected_index:%d, in_editing:%d",
             selected_index,
             selected_in_editing_property);

    if (selected_in_editing_property) {
        GetSelectedElement()->SelectNext();
        return;
    }

    DetachElement((*this)[selected_index]);
    selected_index--;
    if (selected_index < 0) {
        selected_index = size() - 1;
    }
    force_do_action_result = true;
}

void ElementsBox::PageUp() {
    bool selected_in_editing_property =
        GetSelectedElement()->Editing() && GetSelectedElement()->InEditingProperty();
    ESP_LOGI(TAG_ElementsBox,
             "PageUp, selected_index:%d, in_editing:%d",
             selected_index,
             selected_in_editing_property);

    if (selected_in_editing_property) {
        GetSelectedElement()->PageUp();
    } else {
        SelectPrior();
    }
}

void ElementsBox::PageDown() {
    bool selected_in_editing_property =
        GetSelectedElement()->Editing() && GetSelectedElement()->InEditingProperty();
    ESP_LOGI(TAG_ElementsBox,
             "PageDown, selected_index:%d, in_editing:%d",
             selected_index,
             selected_in_editing_property);

    if (selected_in_editing_property) {
        GetSelectedElement()->PageDown();
    } else {
        SelectNext();
    }
}

void ElementsBox::Change() {
    bool selected_in_editing = GetSelectedElement()->Editing();

    ESP_LOGI(TAG_ElementsBox,
             "Change, selected_index:%d, in_editing:%d",
             selected_index,
             selected_in_editing);

    if (!selected_in_editing) {
        ESP_LOGE(TAG_ElementsBox, "Change");
        return;
    }

    GetSelectedElement()->Change();
}

void ElementsBox::Option() {
    bool selected_in_editing = GetSelectedElement()->Editing();

    ESP_LOGI(TAG_ElementsBox,
             "Option, selected_index:%d, in_editing:%d",
             selected_index,
             selected_in_editing);

    if (!selected_in_editing) {
        ESP_LOGE(TAG_ElementsBox, "Option");
        return;
    }

    GetSelectedElement()->Option();
}

bool ElementsBox::EditingCompleted() {
    bool selected_in_editing = GetSelectedElement()->Editing();
    ESP_LOGI(TAG_ElementsBox,
             "EditingCompleted, selected_index:%d, in_editing:%d",
             selected_index,
             selected_in_editing);
    return !selected_in_editing;
}