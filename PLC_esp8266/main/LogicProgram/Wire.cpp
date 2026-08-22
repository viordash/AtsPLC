#include "LogicProgram/Wire.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lassert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_Wire = "Wire";

Wire::Wire() : LogicElement() {
    this->width = 0;
}

Wire::~Wire() {
}

uint8_t Wire::GetWidth() {
    return this->width;
}

void Wire::SetWidth(uint8_t width) {
    this->width = width;
}

bool Wire::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    state = prev_elem_state;
    return prev_elem_changed;
}

IRAM_ATTR void Wire::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {
    uint8_t l_width;
    EditableElement::ElementState l_editable_state;
    {
        std::lock_guard<std::mutex> lock(lock_mutex);
        l_width = width;
        l_editable_state = editable_state;
    }

    ESP_LOGD(TAG_Wire, "Render w:%u", l_width);
    if (l_width == 0) {
        return;
    }

    uint8_t income_width = 0;
    if (l_width > WIRE_BLINK_BODY_WIDTH) {
        income_width = l_width - WIRE_BLINK_BODY_WIDTH;
        bool prev_elem_active = prev_elem_state == LogicItemState::lisActive
                             || prev_elem_state == LogicItemState::lisStop;
        if (prev_elem_active) {
            ASSERT(draw_active_network(fb, start_point->x, start_point->y, income_width));
        } else {
            ASSERT(draw_passive_network(fb, start_point->x, start_point->y, income_width, false));
        }

        start_point->x += income_width;
    }
    uint8_t body_width = l_width - income_width;

    bool blink_on_editing =
        l_editable_state == EditableElement::ElementState::des_Editing && Blinking_50(fb);

    if (!blink_on_editing) {
        bool prev_elem_active = prev_elem_state == LogicItemState::lisActive
                             || prev_elem_state == LogicItemState::lisStop;
        if (prev_elem_active) {
            ASSERT(draw_active_network(fb, start_point->x, start_point->y, body_width));
        } else {
            ASSERT(draw_passive_network(fb, start_point->x, start_point->y, body_width, false));
        }
    }

    start_point->x += body_width;
    EditableElement::Render(fb, start_point);
}

size_t Wire::Serialize(uint8_t *buffer, size_t buffer_size) {
    (void)buffer;
    (void)buffer_size;
    return 0;
}

size_t Wire::Deserialize(uint8_t *buffer, size_t buffer_size) {
    (void)buffer;
    (void)buffer_size;
    return 0;
}

TvElementType Wire::GetElementType() {
    return TvElementType::et_Wire;
}

void Wire::SelectPrior() {
}

void Wire::SelectNext() {
}

void Wire::PageUp() {
}

void Wire::PageDown() {
}

void Wire::Change() {
    EndEditing();
}

void Wire::Option() {
}

Wire *Wire::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_Wire:
            return static_cast<Wire *>(logic_element);

        default:
            return NULL;
    }
}