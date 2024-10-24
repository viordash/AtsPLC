#include "LogicProgram/Wire.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_err.h"
#include "esp_log.h"
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
    (void)prev_elem_state;
    return prev_elem_changed;
}

IRAM_ATTR bool Wire::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = true;
    ESP_LOGI(TAG_Wire, "Render w:%u", width);
    if (width == 0) {
        return true;
    }
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    if (prev_elem_state == LogicItemState::lisActive) {
        res = draw_active_network(fb, start_point->x, start_point->y, width);
    } else {
        res = draw_passive_network(fb, start_point->x, start_point->y, width, false);
    }
    if (!res) {
        return res;
    }

    start_point->x += width;
    res = EditableElement::Render(fb, start_point);
    return res;
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

Wire *Wire::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_Wire:
            return static_cast<Wire *>(logic_element);

        default:
            return NULL;
    }
}