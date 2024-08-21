#include "LogicProgram/Inputs/TimerSecs.h"
#include "Display/bitmaps/timer_sec_active.h"
#include "Display/bitmaps/timer_sec_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *TAG_TimerSecs = "TimerSecs";

TimerSecs::TimerSecs(uint32_t delay_time_s, InputBase *incoming_item) : CommonTimer(incoming_item) {
    if (delay_time_s < 1) {
        delay_time_s = 1;
    }
    if (delay_time_s > 99999) {
        delay_time_s = 99999;
    }
    this->delay_time_us = delay_time_s * 1000000LL;
    str_size = sprintf(this->str_time, "%u", delay_time_s);

    ESP_LOGD(TAG_TimerSecs, "ctor, str_time:%s", this->str_time);
}

TimerSecs::~TimerSecs() {
}

const Bitmap *TimerSecs::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &TimerSecs::bitmap_active;

        default:
            return &TimerSecs::bitmap_passive;
    }
}

bool TimerSecs::Render(uint8_t *fb, LogicItemState state) {
    bool res;
    res = CommonTimer::Render(fb, state);

    uint8_t x_pos = incoming_point.x + LeftPadding - VERT_PROGRESS_BAR_WIDTH;
    uint8_t percent = GetProgress();
    res &= draw_vert_progress_bar(fb,
                                  x_pos,
                                  incoming_point.y - (VERT_PROGRESS_BAR_HEIGHT + 1),
                                  percent);

    ESP_LOGD(TAG_TimerSecs,
             "Render, percent:%u, delay:%u",
             percent,
             (uint32_t)(delay_time_us / 1000000LL));
    return res;
}

bool TimerSecs::ProgressHasChanges() {
    if (incoming_item->GetState() != LogicItemState::lisActive) {
        return false;
    }
    if (state == LogicItemState::lisActive) {
        return false;
    }

    uint64_t curr_time = esp_timer_get_time();
    int64_t elapsed = curr_time - force_render_time_us;
    if (elapsed >= 0 && elapsed < (int64_t)force_render_period_us) {
        return false;
    }
    if (elapsed < 0 && elapsed > (int64_t)-force_render_period_us) {
        return false;
    }
    force_render_time_us = curr_time;
    return true;
}