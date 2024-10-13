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
    res = draw_text_f6X12(fb, start_point->x, start_point->y - LabeledLogicItem::height, label);
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

void CommonInput::SelectNext() {
}
void CommonInput::SelectPrior() {
}
void CommonInput::Change() {
}
bool CommonInput::EditingCompleted() {
    return true;
}