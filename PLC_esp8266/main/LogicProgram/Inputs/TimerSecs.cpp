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

TimerSecs::TimerSecs(uint32_t delay_time_s)
    : CommonTimer() {
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

bool TimerSecs::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    bool any_changes = CommonTimer::DoAction(prev_elem_changed, prev_elem_state);

    if (!any_changes) {
        any_changes = ProgressHasChanges(prev_elem_state);
    }
    return any_changes;
}

bool TimerSecs::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res;
    res = CommonTimer::Render(fb, state, start_point);

    uint8_t x_pos = start_point->x + LeftPadding - VERT_PROGRESS_BAR_WIDTH;
    uint8_t percent = GetProgress(prev_elem_state);
    res &= draw_vert_progress_bar(fb,
                                  x_pos,
                                  start_point->y - (VERT_PROGRESS_BAR_HEIGHT + 1),
                                  percent);

    ESP_LOGD(TAG_TimerSecs,
             "Render, percent:%u, delay:%u",
             percent,
             (uint32_t)(delay_time_us / 1000000LL));
    return res;
}

bool TimerSecs::ProgressHasChanges(LogicItemState prev_elem_state) {
    if (prev_elem_state != LogicItemState::lisActive) {
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