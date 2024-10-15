#include "LogicProgram/Inputs/CommonInput.h"
#include "LogicProgram/Inputs/ComparatorEq.h"
#include "LogicProgram/Inputs/ComparatorGE.h"
#include "LogicProgram/Inputs/ComparatorGr.h"
#include "LogicProgram/Inputs/ComparatorLE.h"
#include "LogicProgram/Inputs/ComparatorLs.h"
#include "LogicProgram/Inputs/InputNC.h"
#include "LogicProgram/Inputs/InputNO.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_CommonInput = "CommonInput";

CommonInput::CommonInput() : LogicElement(), InputElement() {
}

CommonInput::~CommonInput() {
}

void CommonInput::SetIoAdr(const MapIO io_adr) {
    InputElement::SetIoAdr(io_adr);
    SetLabel(MapIONames[io_adr]);
}

IRAM_ATTR bool
CommonInput::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = true;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    auto bitmap = GetCurrentBitmap(state);

    if (prev_elem_state == LogicItemState::lisActive) {
        res = draw_active_network(fb,
                                  start_point->x,
                                  start_point->y,
                                  LabeledLogicItem::width + LeftPadding);
    } else {
        res = draw_passive_network(fb,
                                   start_point->x,
                                   start_point->y,
                                   LabeledLogicItem::width + LeftPadding,
                                   false);
    }
    if (!res) {
        return res;
    }

    start_point->x += LeftPadding;

    bool blink_label_on_editing = editable_state == EditableElement::ElementState::des_Editing
                               && (CommonInput::EditingPropertyId)editing_property_id
                                      == CommonInput::EditingPropertyId::ciepi_ConfigureInputAdr
                               && (esp_timer_get_time() & blink_timer_524ms);
    res = blink_label_on_editing
       || draw_text_f6X12(fb, start_point->x, start_point->y - LabeledLogicItem::height, label);
    if (!res) {
        return res;
    }

    start_point->x += LabeledLogicItem::width;
    draw_bitmap(fb, start_point->x, start_point->y - (bitmap->size.height / 2) + 1, bitmap);

    start_point->x += bitmap->size.width;

    res = EditableElement::Render(fb, start_point);
    return res;
}

CommonInput *CommonInput::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_InputNC:
        case TvElementType::et_InputNO:
        case TvElementType::et_ComparatorEq:
        case TvElementType::et_ComparatorGE:
        case TvElementType::et_ComparatorGr:
        case TvElementType::et_ComparatorLE:
        case TvElementType::et_ComparatorLs:
            return static_cast<CommonInput *>(logic_element);

        default:
            return NULL;
    }
}

void CommonInput::BeginEditing() {
    EditableElement::BeginEditing();
    editing_property_id = CommonInput::EditingPropertyId::ciepi_ConfigureInputAdr;
}

void CommonInput::SelectPrior() {
    ESP_LOGI(TAG_CommonInput, "SelectPrior");

    auto allowed_inputs = GetAllowedInputs();
    auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
    io_adr--;
    if (io_adr < 0) {
        io_adr = allowed_inputs.count - 1;
    }
    SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
}

void CommonInput::SelectNext() {
    ESP_LOGI(TAG_CommonInput, "SelectNext");

    auto allowed_inputs = GetAllowedInputs();
    auto io_adr = FindAllowedIO(&allowed_inputs, GetIoAdr());
    io_adr++;
    if (io_adr >= (int)allowed_inputs.count) {
        io_adr = 0;
    }
    SetIoAdr(allowed_inputs.inputs_outputs[io_adr]);
}

void CommonInput::PageUp() {
}

void CommonInput::PageDown() {
}

void CommonInput::Change() {
    ESP_LOGI(TAG_CommonInput, "Change");
    EndEditing();
}

bool CommonInput::EditingCompleted() {
    ESP_LOGI(TAG_CommonInput, "EditingCompleted");
    return true;
}