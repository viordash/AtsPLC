#include "LogicProgram/Inputs/WiFiInput.h"
#include "Display/bitmaps/wifi_input_active.h"
#include "Display/bitmaps/wifi_input_passive.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_WiFiInput = "WiFiInput";

WiFiInput::WiFiInput() : CommonInput() {
}

WiFiInput::WiFiInput(const MapIO io_adr) : WiFiInput() {
    SetIoAdr(io_adr);
}

WiFiInput::~WiFiInput() {
}

bool WiFiInput::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive //
        && GetValue() == LogicElement::MinValue) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        any_changes = true;
        ESP_LOGD(TAG_WiFiInput, ".");
    }
    return any_changes;
}

const Bitmap *WiFiInput::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &WiFiInput::bitmap_active;

        default:
            return &WiFiInput::bitmap_passive;
    }
}

size_t WiFiInput::Serialize(uint8_t *buffer, size_t buffer_size) {
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

size_t WiFiInput::Deserialize(uint8_t *buffer, size_t buffer_size) {
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

TvElementType WiFiInput::GetElementType() {
    return TvElementType::et_WiFiInput;
}

WiFiInput *WiFiInput::TryToCast(CommonInput *common_input) {
    switch (common_input->GetElementType()) {
        case TvElementType::et_WiFiInput:
            return static_cast<WiFiInput *>(common_input);

        default:
            return NULL;
    }
}

const AllowedIO WiFiInput::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::DI, MapIO::AI, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}