#include "LogicProgram/Inputs/CommonComparator.h"
#include "LogicProgram/Inputs/ComparatorEq.h"
#include "LogicProgram/Inputs/ComparatorGE.h"
#include "LogicProgram/Inputs/ComparatorGr.h"
#include "LogicProgram/Inputs/ComparatorLE.h"
#include "LogicProgram/Inputs/ComparatorLs.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_CommonComparator = "CommonComparator";

CommonComparator::CommonComparator() : CommonInput() {
}

CommonComparator::CommonComparator(uint8_t ref_percent04, const MapIO io_adr) : CommonInput() {
    SetReference(ref_percent04);
    SetIoAdr(io_adr);
}

CommonComparator::~CommonComparator() {
}

void CommonComparator::SetReference(uint8_t ref_percent04) {
    if ((int)ref_percent04 > (int)LogicElement::MaxValue) {
        ref_percent04 = LogicElement::MaxValue;
    }
    this->ref_percent04 = ref_percent04;
    str_size = sprintf(this->str_reference, "%d", ref_percent04);
}

uint8_t CommonComparator::GetReference() {
    return ref_percent04;
}

bool CommonComparator::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive //
        && this->CompareFunction()) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        any_changes = true;
    }
    return any_changes;
}

IRAM_ATTR bool
CommonComparator::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    bool res;

    uint8_t x_pos = start_point->x + LeftPadding + 2;

    res = CommonInput::Render(fb, prev_elem_state, start_point);
    if (!res) {
        return res;
    }

    bool blink_label_on_editing =
        editable_state == EditableElement::ElementState::des_Editing
        && (CommonComparator::EditingPropertyId)editing_property_id
               == CommonComparator::EditingPropertyId::ccepi_ConfigureReference
        && Blinking_50() && label_width > 0;
    switch (str_size) {
        case 1:
            res = blink_label_on_editing
               || (draw_text_f5X7(fb, x_pos + 3 + label_width, start_point->y + 2, str_reference)
                   > 0);
            break;
        case 2:
            res = blink_label_on_editing
               || (draw_text_f5X7(fb, x_pos + 0 + label_width, start_point->y + 2, str_reference)
                   > 0);
            break;
        default:
            res = blink_label_on_editing
               || (draw_text_f4X7(fb, x_pos + label_width, start_point->y + 3, str_reference) > 0);
            break;
    }

    return res;
}

size_t CommonComparator::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&ref_percent04, sizeof(ref_percent04), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&io_adr, sizeof(io_adr), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t CommonComparator::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint8_t _ref_percent04;
    if (!Record::Read(&_ref_percent04, sizeof(_ref_percent04), buffer, buffer_size, &readed)) {
        return 0;
    }
    if ((int)_ref_percent04 > (int)LogicElement::MaxValue) {
        return 0;
    }

    MapIO _io_adr;
    if (!Record::Read(&_io_adr, sizeof(_io_adr), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateMapIO(_io_adr)) {
        return 0;
    }
    SetIoAdr(_io_adr);
    SetReference(_ref_percent04);
    return readed;
}

CommonComparator *CommonComparator::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_ComparatorEq:
        case TvElementType::et_ComparatorGE:
        case TvElementType::et_ComparatorGr:
        case TvElementType::et_ComparatorLE:
        case TvElementType::et_ComparatorLs:
            return static_cast<CommonComparator *>(logic_element);

        default:
            return NULL;
    }
}

void CommonComparator::SelectPrior() {
    uint8_t ref;
    switch (editing_property_id) {
        case CommonComparator::EditingPropertyId::ccepi_ConfigureIoAdr:
            CommonInput::SelectPrior();
            return;

        case CommonComparator::EditingPropertyId::ccepi_ConfigureReference:
            ref = GetReference();
            if (ref <= LogicElement::MaxValue - step_ref) {
                SetReference(ref + step_ref);
            } else {
                SetReference(LogicElement::MaxValue);
            }
            break;

        default:
            break;
    }
}

void CommonComparator::SelectNext() {
    uint8_t ref;
    switch (editing_property_id) {
        case CommonComparator::EditingPropertyId::ccepi_ConfigureIoAdr:
            CommonInput::SelectNext();
            return;

        case CommonComparator::EditingPropertyId::ccepi_ConfigureReference:
            ref = GetReference();
            if (ref >= LogicElement::MinValue + step_ref) {
                SetReference(ref - step_ref);
            } else {
                SetReference(LogicElement::MinValue);
            }
            break;

        default:
            break;
    }
}

void CommonComparator::PageUp() {
    uint8_t ref;
    switch (editing_property_id) {
        case CommonComparator::EditingPropertyId::ccepi_ConfigureReference:
            ref = GetReference();
            if (ref <= LogicElement::MaxValue - faststep_ref) {
                SetReference(ref + faststep_ref);
            } else {
                SetReference(LogicElement::MaxValue);
            }
            break;

        default:
            break;
    }
}

void CommonComparator::PageDown() {
    uint8_t ref;
    switch (editing_property_id) {
        case CommonComparator::EditingPropertyId::ccepi_ConfigureReference:
            ref = GetReference();
            if (ref >= LogicElement::MinValue + faststep_ref) {
                SetReference(ref - faststep_ref);
            } else {
                SetReference(LogicElement::MinValue);
            }
            break;

        default:
            break;
    }
}

void CommonComparator::Change() {
    switch (editing_property_id) {
        case CommonComparator::EditingPropertyId::ccepi_ConfigureIoAdr:
            ESP_LOGI(TAG_CommonComparator, "Change");
            editing_property_id = CommonComparator::EditingPropertyId::ccepi_ConfigureReference;
            break;

        default:
            CommonInput::Change();
            return;
    }
}