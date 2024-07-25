#include "DisplayItemBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DisplayItemBase::DisplayItemBase(const Point &location, const Size &size) {
    this->location = location;
    this->size = size;
}

DisplayItemBase::~DisplayItemBase() {
}

const Point &DisplayItemBase::GetLocation() {
    return location;
}

const Size &DisplayItemBase::GetSize() {
    return size;
}
