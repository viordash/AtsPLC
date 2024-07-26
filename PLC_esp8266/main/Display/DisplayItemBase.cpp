#include "Display/DisplayItemBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DisplayItemBase::DisplayItemBase(const Point &location) {
    this->location = location;
}

DisplayItemBase::~DisplayItemBase() {
}

const Point &DisplayItemBase::GetLocation() {
    return location;
}
