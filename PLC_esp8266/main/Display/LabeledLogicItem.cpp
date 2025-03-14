#include "Display/LabeledLogicItem.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void LabeledLogicItem::SetLabel(const char *label) {
    this->label = label;
    this->label_width = 0;
}

const char *LabeledLogicItem::GetLabel() {
    return this->label;
}