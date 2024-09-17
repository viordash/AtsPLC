#include "Display/SelectableElement.h"
#include "Display/display.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_SelectableElement = "SelectableElement";

SelectableElement::SelectableElement() {
    selected = false;
}

SelectableElement::~SelectableElement() {
}

void SelectableElement::ChangeSelection(bool selected) {
    ESP_LOGD(TAG_SelectableElement, "ChangeSelection, %u", selected);
    this->selected = selected;
}

bool SelectableElement::Selected() {
    return selected;
}