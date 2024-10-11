
#include "LogicProgram/ElementsBox.h"
#include "Display/display.h"
#include "LogicProgram/Inputs/CommonComparator.h"
#include "LogicProgram/Inputs/CommonTimer.h"
#include "LogicProgram/Inputs/InputNC.h"
#include "LogicProgram/Inputs/InputNO.h"
#include "LogicProgram/Inputs/TimerMSecs.h"
#include "LogicProgram/Inputs/TimerSecs.h"
#include "LogicProgram/Serializer/LogicElementFactory.h"
#include "esp_err.h"
#include "esp_log.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ElementsBox = "ElementsBox";

ElementsBox::ElementsBox(uint8_t place_width, LogicElement *stored_element) {
    this->place_width = place_width;
    this->stored_element = stored_element;
    selected_index = -1;
    Fill();
}

ElementsBox::~ElementsBox() {
    if (Editing()) {
        delete stored_element;
    } else if (GetSelectedElement() != stored_element) {
        auto selected_it = std::find(begin(), end(), GetSelectedElement());
        erase(selected_it);
        delete stored_element;
    }

    while (!empty()) {
        auto it = begin();
        auto element = *it;
        erase(it);
        ESP_LOGD(TAG_ElementsBox, "delete elem: %p", element);
        delete element;
    }
}

bool ElementsBox::MatchedToStoredElement(TvElementType element_type) {
    if (IsInputElement(element_type) && IsOutputElement(stored_element->GetElementType())) {
        return false;
    }

    if (IsOutputElement(element_type) && IsInputElement(stored_element->GetElementType())) {
        return false;
    }
    return true;
}

bool ElementsBox::CopyParamsToCommonInput(CommonInput *common_input) {
    if (common_input == NULL) {
        return false;
    }
    MapIO io_adr = MapIO::AI;

    auto *stored_element_as_commonInput = CommonInput::TryToCast(stored_element);
    if (stored_element_as_commonInput != NULL) {
        io_adr = stored_element_as_commonInput->GetIoAdr();
    } else {
        auto *stored_element_as_commonOutput = CommonOutput::TryToCast(stored_element);
        if (stored_element_as_commonOutput != NULL) {
            io_adr = stored_element_as_commonOutput->GetIoAdr();
        }
    }

    auto *new_element_as_commonComparator = CommonComparator::TryToCast(common_input);
    if (new_element_as_commonComparator != NULL) {
        uint8_t ref_percent04 = 0;

        if (stored_element_as_commonInput != NULL) {
            auto *stored_element_as_commonComparator =
                CommonComparator::TryToCast(stored_element_as_commonInput);

            if (stored_element_as_commonComparator != NULL) {
                ref_percent04 = stored_element_as_commonComparator->GetReference();
            }
        }
        new_element_as_commonComparator->SetReference(ref_percent04);
    }
    common_input->SetIoAdr(io_adr);
    return true;
}

bool ElementsBox::CopyParamsToCommonTimer(CommonTimer *common_timer) {
    if (common_timer == NULL) {
        return false;
    }

    uint64_t time_us = 0;
    auto *stored_element_as_commonTimer = CommonTimer::TryToCast(stored_element);
    if (stored_element_as_commonTimer != NULL) {
        auto *stored_element_as_timerSecs = TimerSecs::TryToCast(stored_element_as_commonTimer);
        if (stored_element_as_timerSecs != NULL) {
            time_us = stored_element_as_timerSecs->GetTimeUs();
        } else {
            auto *stored_element_as_timerMSecs =
                TimerMSecs::TryToCast(stored_element_as_commonTimer);
            if (stored_element_as_timerMSecs != NULL) {
                time_us = stored_element_as_timerMSecs->GetTimeUs();
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

bool ElementsBox::CopyParamsToCommonOutput(CommonOutput *common_output) {
    if (common_output == NULL) {
        return false;
    }

    MapIO io_adr = MapIO::V1;
    auto *stored_element_as_commonInput = CommonInput::TryToCast(stored_element);
    if (stored_element_as_commonInput != NULL) {
        io_adr = stored_element_as_commonInput->GetIoAdr();
    } else {
        auto *stored_element_as_commonOutput = CommonOutput::TryToCast(stored_element);
        if (stored_element_as_commonOutput != NULL) {
            io_adr = stored_element_as_commonOutput->GetIoAdr();
        }
    }
    common_output->SetIoAdr(io_adr);
    return true;
}

void ElementsBox::TakeParamsFromStoredElement(LogicElement *new_element) {
    if (CopyParamsToCommonInput(CommonInput::TryToCast(new_element))) {
        return;
    }
    if (CopyParamsToCommonTimer(CommonTimer::TryToCast(new_element))) {
        return;
    }
    if (CopyParamsToCommonOutput(CommonOutput::TryToCast(new_element))) {
        return;
    }
}

void ElementsBox::AppendStandartElement(TvElementType element_type, uint8_t *frame_buffer) {
    if (stored_element->GetElementType() == element_type) {
        return;
    }

    if (!MatchedToStoredElement(element_type)) {
        return;
    }

    auto new_element = LogicElementFactory::Create(element_type);
    if (new_element == NULL) {
        return;
    }
    TakeParamsFromStoredElement(new_element);

    Point start_point = { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 };
    if (!new_element->Render(frame_buffer, LogicItemState::lisPassive, &start_point)) {
        delete new_element;
        return;
    }
    bool element_not_fit = start_point.x - (DISPLAY_WIDTH / 2) > place_width;
    if (element_not_fit) {
        delete new_element;
        return;
    }
    new_element->SkipEditableStateRendering();
    push_back(new_element);
}

void ElementsBox::Fill() {
    uint8_t *frame_buffer = new uint8_t[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];

    AppendStandartElement(TvElementType::et_InputNC, frame_buffer);
    AppendStandartElement(TvElementType::et_InputNO, frame_buffer);
    AppendStandartElement(TvElementType::et_TimerSecs, frame_buffer);
    AppendStandartElement(TvElementType::et_TimerMSecs, frame_buffer);
    AppendStandartElement(TvElementType::et_ComparatorEq, frame_buffer);
    AppendStandartElement(TvElementType::et_ComparatorGE, frame_buffer);
    AppendStandartElement(TvElementType::et_ComparatorGr, frame_buffer);
    AppendStandartElement(TvElementType::et_ComparatorLE, frame_buffer);
    AppendStandartElement(TvElementType::et_ComparatorLs, frame_buffer);
    AppendStandartElement(TvElementType::et_DirectOutput, frame_buffer);
    AppendStandartElement(TvElementType::et_SetOutput, frame_buffer);
    AppendStandartElement(TvElementType::et_ResetOutput, frame_buffer);
    AppendStandartElement(TvElementType::et_IncOutput, frame_buffer);
    AppendStandartElement(TvElementType::et_DecOutput, frame_buffer);

    delete[] frame_buffer;
}

LogicElement *ElementsBox::GetSelectedElement() {
    if (selected_index < 0) {
        return stored_element;
    }
    return (*this)[selected_index];
}

bool ElementsBox::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    return GetSelectedElement()->DoAction(prev_elem_changed, prev_elem_state);
}

bool ElementsBox::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    return GetSelectedElement()->Render(fb, prev_elem_state, start_point);
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

void ElementsBox::HandleButtonUp() {
    bool selected_in_editing = GetSelectedElement()->Editing();

    ESP_LOGI(TAG_ElementsBox,
             "HandleButtonUp, selected_index:%d, in_editing:%d",
             selected_index,
             selected_in_editing);

    if (selected_in_editing) {
        SelectedElementHandleButtonUp();
        return;
    }

    selected_index--;

    if (selected_index < -1) {
        selected_index = size() - 1;
    }
}

void ElementsBox::HandleButtonDown() {
    bool selected_in_editing = GetSelectedElement()->Editing();

    ESP_LOGI(TAG_ElementsBox,
             "HandleButtonDown, selected_index:%d, in_editing:%d",
             selected_index,
             selected_in_editing);

    if (selected_in_editing) {
        SelectedElementHandleButtonDown();
        return;
    }

    selected_index++;
    if (selected_index >= (int)size()) {
        selected_index = -1;
    }
}
void ElementsBox::HandleButtonSelect() {
    bool selected_in_editing = GetSelectedElement()->Editing();

    ESP_LOGI(TAG_ElementsBox,
             "HandleButtonSelect, selected_index:%d, in_editing:%d",
             selected_index,
             selected_in_editing);

    if (selected_in_editing) {
        SelectedElementHandleButtonSelect();
        return;
    }
    GetSelectedElement()->BeginEditing();
}

void ElementsBox::SelectedElementHandleButtonUp() {
    ESP_LOGI(TAG_ElementsBox, "SelectedElementHandleButtonUp, selected_index:%d", selected_index);
}

void ElementsBox::SelectedElementHandleButtonDown() {
    ESP_LOGI(TAG_ElementsBox, "SelectedElementHandleButtonDown, selected_index:%d", selected_index);
}

void ElementsBox::SelectedElementHandleButtonSelect() {
    ESP_LOGI(TAG_ElementsBox,
             "SelectedElementHandleButtonSelect, selected_index:%d",
             selected_index);

    GetSelectedElement()->EndEditing();
    GetSelectedElement()->Select();
}

void ElementsBox::EndEditing() {
    ESP_LOGI(TAG_ElementsBox, "EndEditing, %u", (unsigned)editable_state);

    GetSelectedElement()->EndEditing();
    EditableElement::EndEditing();
}