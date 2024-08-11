#include "Display/DisplayChainItem.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DisplayChainItem::DisplayChainItem(const Point &incoming_point,
                                   LogicItemState incoming_item_state) {
    this->incoming_point = incoming_point;
    this->incoming_item_state = incoming_item_state;
}

DisplayChainItem::DisplayChainItem(DisplayChainItem *incoming_item)
    : DisplayChainItem(incoming_item->OutcomingPoint(), incoming_item->incoming_item_state) {
}

DisplayChainItem::~DisplayChainItem() {
}