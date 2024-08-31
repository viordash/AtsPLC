#include "LogicProgram/Inputs/TimerMSecs.h"
#include "Display/bitmaps/timer_msec_active.h"
#include "Display/bitmaps/timer_msec_passive.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *TAG_TimerMSecs = "TimerMSecs";

TimerMSecs::TimerMSecs(uint32_t delay_time_ms)
    : CommonTimer() {
    if (delay_time_ms < 1) {
        delay_time_ms = 1;
    }
    if (delay_time_ms > 99999) {
        delay_time_ms = 99999;
    }
    this->delay_time_us = delay_time_ms * 1000LL;
    str_size = sprintf(this->str_time, "%u", delay_time_ms);

    ESP_LOGD(TAG_TimerMSecs, "ctor, str_time:%s", this->str_time);
}

TimerMSecs::~TimerMSecs() {
}

const Bitmap *TimerMSecs::GetCurrentBitmap() {
    switch (state) {
        case LogicItemState::lisActive:
            return &TimerMSecs::bitmap_active;

        default:
            return &TimerMSecs::bitmap_passive;
    }
}

size_t TimerMSecs::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;

    return writed;
}

size_t TimerMSecs::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;

    return readed;
}