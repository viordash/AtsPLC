#include "LogicProgram/Inputs/CommonTimer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *TAG_CommonTimer = "CommonTimer";

CommonTimer::CommonTimer(InputBase *incoming_item)
    : InputBase(incoming_item->controller, incoming_item->OutcomingPoint()) {
    this->incoming_item = incoming_item;
    this->incoming_item->Bind(this);
    incoming_item_prev_state = incoming_item->GetState();
}

CommonTimer::~CommonTimer() {
}

uint64_t CommonTimer::GetLeftTime() {
    uint64_t curr_time = esp_timer_get_time();
    uint64_t left_time;
    if (raise_time_us >= curr_time) {
        left_time = raise_time_us - curr_time;
    } else {
        left_time = raise_time_us + 1 + (UINT64_MAX - curr_time);
    }

    if (left_time > delay_time_us) {
        left_time = 0;
    }
    return left_time;
}

uint8_t CommonTimer::GetProgress() {
    uint64_t left_time = GetLeftTime();
    uint8_t percent = (left_time * 100) / delay_time_us;
    return 100 - (uint8_t)percent;
}

bool CommonTimer::Render(uint8_t *fb) {
    bool res = true;
    auto bitmap = GetCurrentBitmap();

    if (incoming_item->GetState() == LogicItemState::lisActive) {
        res &= draw_active_network(fb, incoming_point.x, incoming_point.y, LeftPadding);
    } else {
        res &= draw_passive_network(fb, incoming_point.x, incoming_point.y, LeftPadding, false);
    }

    uint8_t x_pos = incoming_point.x + LeftPadding;

    draw_bitmap(fb, x_pos, incoming_point.y - (bitmap->size.height / 2) + 1, bitmap);

    switch (str_size) {
        case 1:
            res &= draw_text_f5X7(fb, x_pos + 10, incoming_point.y + 2, str_time);
            break;
        case 2:
            res &= draw_text_f5X7(fb, x_pos + 6, incoming_point.y + 2, str_time);
            break;
        case 3:
            res &= draw_text_f5X7(fb, x_pos + 3, incoming_point.y + 2, str_time);
            break;
        case 4:
            res &= draw_text_f4X7(fb, x_pos + 4, incoming_point.y + 3, str_time);
            break;
        default:
            res &= draw_text_f4X7(fb, x_pos + 2, incoming_point.y + 3, str_time);
            break;
    }

    x_pos += bitmap->size.width;
    if (state == LogicItemState::lisActive) {
        res &= draw_active_network(fb, x_pos, incoming_point.y, RightPadding);
    } else {
        res &= draw_passive_network(fb, x_pos, incoming_point.y, RightPadding, true);
    }

    ESP_LOGI(TAG_CommonTimer,
             "Render, str_time:%s, str_size:%d, x:%u, y:%u",
             str_time,
             str_size,
             incoming_point.x,
             incoming_point.y);
    return res;
}

Point CommonTimer::OutcomingPoint() {
    auto bitmap = GetCurrentBitmap();
    uint8_t x_pos = incoming_point.x + LeftPadding + bitmap->size.width + RightPadding;
    uint8_t y_pos = incoming_point.y;
    return { x_pos, y_pos };
}

bool CommonTimer::IncomingItemStateHasChanged() {
    bool changed = incoming_item_prev_state != incoming_item->GetState();
    incoming_item_prev_state = incoming_item->GetState();
    return changed;
}