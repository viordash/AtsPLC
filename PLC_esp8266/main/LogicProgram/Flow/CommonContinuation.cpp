#include "LogicProgram/Flow/CommonContinuation.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_CommonContinuation = "CommonContinuation";

CommonContinuation::CommonContinuation() : LogicElement() {
}

CommonContinuation::~CommonContinuation() {
}

bool CommonContinuation::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {

        any_changes = true;
        ESP_LOGD(TAG_CommonContinuation, ".");
    }

    return any_changes;
}

size_t CommonContinuation::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    uint8_t dummy = 0;
    if (!Record::Write(&dummy, sizeof(dummy), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t CommonContinuation::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint8_t dummy;
    if (!Record::Read(&dummy, sizeof(dummy), buffer, buffer_size, &readed)) {
        return 0;
    }
    return readed;
}

void CommonContinuation::SelectPrior() {
}

void CommonContinuation::SelectNext() {
}

void CommonContinuation::PageUp() {
}

void CommonContinuation::PageDown() {
}

void CommonContinuation::Change() {
    EndEditing();
}

void CommonContinuation::Option() {
}
