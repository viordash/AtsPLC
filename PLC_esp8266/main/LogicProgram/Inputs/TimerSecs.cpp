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

static const char *TAG = "TimerSecs";

TimerSecs::TimerSecs(uint32_t delay_time_s, InputBase *incoming_item) : CommonTimer(incoming_item) {
    if (delay_time_s < 1) {
        delay_time_s = 1;
    }
    if (delay_time_s > 99999) {
        delay_time_s = 99999;
    }
    this->delay_time_us = delay_time_s * 1000000LL;
    str_size = sprintf(this->str_time, "%u", delay_time_s);

    ESP_LOGD(TAG, "ctor, str_time:%s", this->str_time);
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

bool TimerSecs::Render(uint8_t *fb) {
    if (!require_render) {
        return true;
    }
    bool res;
    res = CommonTimer::Render(fb);

    uint8_t x_pos = incoming_point.x + LeftPadding - (VERT_PROGRESS_BAR_WIDTH * 2);
    uint8_t percent = GetProgress();
    res &= draw_vert_progress_bar(fb,
                                  x_pos,
                                  incoming_point.y - (VERT_PROGRESS_BAR_HEIGHT + 1),
                                  percent);
    require_render = false;
    return res;
}