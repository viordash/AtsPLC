#include "Display/DisplayItemBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DisplayItemBase::DisplayItemBase() {
    require_render = true;
}

DisplayItemBase::~DisplayItemBase() {
}