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

TimerSecs::TimerSecs(uint16_t delay_time_s, InputBase &prior_item)
    : TimerBase(delay_time_s * 1000000LL, prior_item) {

    str_size = sprintf(this->str_time, "%d", delay_time_s);

    ESP_LOGI(TAG, "ctor, str_time:%s", this->str_time);
}

TimerSecs::~TimerSecs() {
}

bool TimerSecs::DoAction() {
    state =
        state == LogicItemState::lisActive ? LogicItemState::lisPassive : LogicItemState::lisActive;
    return true;
}

const Bitmap *TimerSecs::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &TimerSecs::bitmap_active;

        default:
            return &TimerSecs::bitmap_passive;
    }
}