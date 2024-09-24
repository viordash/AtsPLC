
#include "LogicProgram/ElementsBox.h"
#include "Display/display.h"
#include "LogicProgram/Inputs/InputNC.h"
#include "LogicProgram/Inputs/InputNO.h"
#include "LogicProgram/Serializer/LogicElementFactory.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ElementsBox = "ElementsBox";

ElementsBox::ElementsBox(uint8_t place_width, LogicElement *stored_element) {
    this->place_width = place_width;
    this->stored_element = stored_element;
    Fill();
}

ElementsBox::~ElementsBox() {
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

void ElementsBox::TakeParamsFromStoredElement(LogicElement *new_element) {
    auto *stored_element_as_commonInput = CommonInput::TryToCast(stored_element);
    if (stored_element_as_commonInput != NULL) {
        auto *new_element_as_commonInput = CommonInput::TryToCast(new_element);
        if (new_element_as_commonInput != NULL) {
            new_element_as_commonInput->SetIoAdr(stored_element_as_commonInput->GetIoAdr());
            return;
        }
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