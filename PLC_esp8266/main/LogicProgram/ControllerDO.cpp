#include "LogicProgram/ControllerDO.h"
#include "LogicProgram/LogicElement.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ControllerDO = "ControllerDO";

ControllerDO::ControllerDO(gpio_output gpio) : ControllerBaseInput(), ControllerBaseOutput() {
    this->gpio = gpio;
}

void ControllerDO::Init() {
    ControllerBaseInput::Init();
    ControllerBaseOutput::Init();
    value_changed = false;
}

bool ControllerDO::FetchValue() {
    if (!required_reading) {
        return false;
    }
    required_reading = false;

    bool changed = value_changed;
    value_changed = false;

    bool val_1bit = get_digital_value(gpio);
    uint8_t percent04 = val_1bit ? LogicElement::MaxValue : LogicElement::MinValue;

    ESP_LOGI(TAG_ControllerDO, "FetchValue, percent04:%u, in_value:%u", percent04, PeekValue());

    if (UpdateValue(percent04)) {
        return true;
    }
    return changed;
}

void ControllerDO::CommitChanges() {
    if (!required_writing) {
        return;
    }
    required_writing = false;
    set_digital_value(gpio, out_value != LogicElement::MinValue);
    value_changed = UpdateValue(out_value);
}
