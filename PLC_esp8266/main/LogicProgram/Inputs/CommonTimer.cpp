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
    if (incoming_item->GetState() == LogicItemState::lisActive) {
        this->start_time_us = esp_timer_get_time();
    }
}

CommonTimer::~CommonTimer() {
}

uint64_t CommonTimer::GetLeftTime() {
    if (incoming_item->GetState() != LogicItemState::lisActive) {
        return delay_time_us;
    }
    uint64_t curr_time = esp_timer_get_time();
    int64_t elapsed = curr_time - start_time_us;
    if (elapsed < 0) {
        return 0;
    }

    if (elapsed > (int64_t)delay_time_us) {
        return 0;
    }
    uint64_t left_time = delay_time_us - elapsed;
    return left_time;
}

uint8_t CommonTimer::GetProgress() {
    uint64_t left_time = GetLeftTime();
    uint8_t percent04 = (left_time * StatefulElement::MaxValue) / delay_time_us;
    return StatefulElement::MaxValue - (uint8_t)percent04;
}

bool CommonTimer::DoAction(bool prev_changed) {
    bool any_changes = false;

    LogicItemState prev_state = state;
    if (prev_changed && incoming_item->GetState() == LogicItemState::lisActive) {
        start_time_us = esp_timer_get_time();
    }

    if (incoming_item->GetState() == LogicItemState::lisActive //
        && (state == LogicItemState::lisActive || GetLeftTime() == 0)) {
        state = LogicItemState::lisActive;
    } else {
        state = LogicItemState::lisPassive;
    }

    if (state != prev_state) {
        any_changes = true;
        ESP_LOGD(TAG_CommonTimer, ".");
        
    }
    return any_changes;
}

bool CommonTimer::Render(uint8_t *fb, LogicItemState state) {
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

    ESP_LOGD(TAG_CommonTimer,
             "Render, str_time:%s, str_size:%d, x:%u, y:%u, res:%u",
             str_time,
             str_size,
             incoming_point.x,
             incoming_point.y,
             res);
    return res;
}

Point CommonTimer::OutcomingPoint() {
    auto bitmap = GetCurrentBitmap();
    uint8_t x_pos = incoming_point.x + LeftPadding + bitmap->size.width + RightPadding;
    uint8_t y_pos = incoming_point.y;
    return { x_pos, y_pos };
}