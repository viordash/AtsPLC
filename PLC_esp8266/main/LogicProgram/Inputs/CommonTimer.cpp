#include "LogicProgram/Inputs/CommonTimer.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *TAG_CommonTimer = "CommonTimer";

CommonTimer::CommonTimer() : LogicElement() {
    this->start_time_us = esp_timer_get_time();
    this->delay_time_us = 0;
}

CommonTimer::~CommonTimer() {
}

uint64_t CommonTimer::GetLeftTime() {
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

uint8_t CommonTimer::GetProgress(LogicItemState prev_elem_state) {
    if (prev_elem_state != LogicItemState::lisActive) {
        return LogicElement::MinValue;
    }
    uint64_t left_time = GetLeftTime();
    uint8_t percent04 = (left_time * LogicElement::MaxValue) / delay_time_us;
    return LogicElement::MaxValue - (uint8_t)percent04;
}

bool CommonTimer::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    if (prev_elem_changed && prev_elem_state == LogicItemState::lisActive) {
        start_time_us = esp_timer_get_time();
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state == LogicItemState::lisActive //
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

IRAM_ATTR bool
CommonTimer::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = true;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    auto bitmap = GetCurrentBitmap(state);

    if (prev_elem_state == LogicItemState::lisActive) {
        res = draw_active_network(fb, start_point->x, start_point->y, LeftPadding);
    } else {
        res = draw_passive_network(fb, start_point->x, start_point->y, LeftPadding, false);
    }
    if (!res) {
        return res;
    }

    start_point->x += LeftPadding;

    draw_bitmap(fb, start_point->x, start_point->y - (bitmap->size.height / 2) + 1, bitmap);

    switch (str_size) {
        case 1:
            res = draw_text_f5X7(fb, start_point->x + 10, start_point->y + 2, str_time);
            break;
        case 2:
            res = draw_text_f5X7(fb, start_point->x + 6, start_point->y + 2, str_time);
            break;
        case 3:
            res = draw_text_f5X7(fb, start_point->x + 3, start_point->y + 2, str_time);
            break;
        case 4:
            res = draw_text_f4X7(fb, start_point->x + 4, start_point->y + 3, str_time);
            break;
        default:
            res = draw_text_f4X7(fb, start_point->x + 2, start_point->y + 3, str_time);
            break;
    }

    start_point->x += bitmap->size.width;

    ESP_LOGD(TAG_CommonTimer,
             "Render, str_time:%s, str_size:%d, x:%u, y:%u, res:%u",
             str_time,
             str_size,
             start_point->x,
             start_point->y,
             res);
    return res;
}