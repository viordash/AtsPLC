#include "Display/NetworkedLogicItem.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

NetworkedLogicItem::NetworkedLogicItem(const Point &incoming_point) : DisplayItemBase() {
    this->incoming_point = incoming_point;
}

NetworkedLogicItem::~NetworkedLogicItem() {
}