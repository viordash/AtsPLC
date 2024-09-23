
#include "LogicProgram/ElementsBox.h"
#include "Display/display.h"
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

void ElementsBox::AppendStandartElement(TvElementType element_type, uint8_t *frame_buffer) {
    if (stored_element->GetElementType() == element_type) {
        return;
    }

    if (IsInputElement(element_type) && IsOutputElement(stored_element->GetElementType())) {
        return;
    }

    if (IsOutputElement(element_type) && IsInputElement(stored_element->GetElementType())) {
        return;
    }

    auto element = LogicElementFactory::Create(element_type);
    if (element == NULL) {
        return;
    }

    Point start_point = { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 };
    if (!element->Render(frame_buffer, LogicItemState::lisPassive, &start_point)) {
        return;
    }
    bool element_not_fit = start_point.x - (DISPLAY_WIDTH / 2) > place_width;
    if (element_not_fit) {
        return;
    }
    push_back(element);
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