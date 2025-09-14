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
CommonInput::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = true;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    bool blink_label_on_editing = editable_state == EditableElement::ElementState::des_Editing
                               && (CommonInput::EditingPropertyId)editing_property_id
                                      == CommonInput::EditingPropertyId::ciepi_ConfigureInputAdr
                               && Blinking_50() && label_width > 0;
    if (!blink_label_on_editing) {
        label_width = draw_text_f6X12(fb,
                                      start_point->x + LeftPadding,
                                      start_point->y - get_text_f6X12_height(),
                                      label);
        res = label_width > 0;
    }
    if (!res) {
        return res;
    }

    auto bitmap = GetCurrentBitmap(state);

    if (prev_elem_state == LogicItemState::lisActive) {
        res = draw_active_network(fb, start_point->x, start_point->y, label_width + LeftPadding);
    } else {
        res = draw_passive_network(fb,
                                   start_point->x,
                                   start_point->y,
                                   label_width + LeftPadding,
                                   false);
    }
    if (!res) {
        return res;
    }
    start_point->x += LeftPadding + label_width;

    bool blink_bitmap_on_editing = editable_state == EditableElement::ElementState::des_Editing
                                && (CommonInput::EditingPropertyId)editing_property_id
                                       == CommonInput::EditingPropertyId::ciepi_None
                                && Blinking_50();
    if (!blink_bitmap_on_editing) {
        draw_bitmap(fb, start_point->x, start_point->y - (bitmap->size.height / 2) + 1, bitmap);
    }

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
        case TvElementType::et_ComparatorNe:
            return static_cast<CommonInput *>(logic_element);

        default:
            return NULL;
    }
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
    switch (editing_property_id) {
        case CommonInput::EditingPropertyId::ciepi_None:
            editing_property_id = CommonInput::EditingPropertyId::ciepi_ConfigureInputAdr;
            break;

        default:
            EndEditing();
            break;
    }
}

void CommonInput::Option() {
}