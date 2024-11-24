#include "LogicProgram/Outputs/CommonOutput.h"
#include "Display/display.h"
#include "LogicProgram/Outputs/DecOutput.h"
#include "LogicProgram/Outputs/DirectOutput.h"
#include "LogicProgram/Outputs/IncOutput.h"
#include "LogicProgram/Outputs/ResetOutput.h"
#include "LogicProgram/Outputs/SetOutput.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_CommonOutput = "CommonOutput";

CommonOutput::CommonOutput() : LogicElement(), InputOutputElement() {
}

CommonOutput::~CommonOutput() {
}

void CommonOutput::SetIoAdr(const MapIO io_adr) {
    InputOutputElement::SetIoAdr(io_adr);
    SetLabel(MapIONames[io_adr]);
}

IRAM_ATTR bool
CommonOutput::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    (void)prev_elem_state;
    bool res = true;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    auto bitmap = GetCurrentBitmap(state);

    start_point->x -= LabeledLogicItem::width;

    res = EditableElement::Render(fb, start_point);

    if (res) {
        if (state == LogicItemState::lisActive) {
            res = draw_active_network(fb, start_point->x, start_point->y, LabeledLogicItem::width);
        } else {
            res = draw_passive_network(fb,
                                       start_point->x,
                                       start_point->y,
                                       LabeledLogicItem::width,
                                       true);
        }
    }

    if (res) {
        bool blink_label_on_editing =
            editable_state == EditableElement::ElementState::des_Editing
            && (CommonOutput::EditingPropertyId)editing_property_id
                   == CommonOutput::EditingPropertyId::coepi_ConfigureOutputAdr
            && Blinking_50();
        res = blink_label_on_editing
           || (draw_text_f6X12(fb, start_point->x, start_point->y - get_text_f6X12_height(), label)
               > 0);
    }

    start_point->x -= bitmap->size.width;
    if (res) {
        bool blink_bitmap_on_editing = editable_state == EditableElement::ElementState::des_Editing
                                    && (CommonOutput::EditingPropertyId)editing_property_id
                                           == CommonOutput::EditingPropertyId::coepi_None
                                    && Blinking_50();
        if (!blink_bitmap_on_editing) {
            draw_bitmap(fb, start_point->x, start_point->y - (bitmap->size.height / 2) + 1, bitmap);
        }
    }

    return res;
}

size_t CommonOutput::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&io_adr, sizeof(io_adr), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t CommonOutput::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    MapIO _io_adr;
    if (!Record::Read(&_io_adr, sizeof(_io_adr), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateMapIO(_io_adr)) {
        return 0;
    }
    SetIoAdr(_io_adr);
    return readed;
}

CommonOutput *CommonOutput::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_DirectOutput:
        case TvElementType::et_SetOutput:
        case TvElementType::et_ResetOutput:
        case TvElementType::et_IncOutput:
        case TvElementType::et_DecOutput:
            return static_cast<CommonOutput *>(logic_element);

        default:
            return NULL;
    }
}

void CommonOutput::SelectPrior() {
    ESP_LOGI(TAG_CommonOutput, "SelectPrior");

    auto allowed_outputs = GetAllowedOutputs();
    auto io_adr = FindAllowedIO(&allowed_outputs, GetIoAdr());
    io_adr--;
    if (io_adr < 0) {
        io_adr = allowed_outputs.count - 1;
    }
    SetIoAdr(allowed_outputs.inputs_outputs[io_adr]);
}

void CommonOutput::SelectNext() {
    ESP_LOGI(TAG_CommonOutput, "SelectNext");

    auto allowed_outputs = GetAllowedOutputs();
    auto io_adr = FindAllowedIO(&allowed_outputs, GetIoAdr());
    io_adr++;
    if (io_adr >= (int)allowed_outputs.count) {
        io_adr = 0;
    }
    SetIoAdr(allowed_outputs.inputs_outputs[io_adr]);
}

void CommonOutput::PageUp() {
}
void CommonOutput::PageDown() {
}

void CommonOutput::Change() {
    ESP_LOGI(TAG_CommonOutput, "Change");
    switch (editing_property_id) {
        case CommonOutput::EditingPropertyId::coepi_None:
            editing_property_id = CommonOutput::EditingPropertyId::coepi_ConfigureOutputAdr;
            break;

        default:
            EndEditing();
            break;
    }
}