#include "LogicProgram/Inputs/TimerMSecs.h"
#include "Display/bitmaps/timer_msec_active.h"
#include "Display/bitmaps/timer_msec_passive.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *TAG_TimerMSecs = "TimerMSecs";

TimerMSecs::TimerMSecs() : CommonTimer() {
}

TimerMSecs::TimerMSecs(uint32_t delay_time_ms) : TimerMSecs() {
    SetTime(delay_time_ms);
}

TimerMSecs::~TimerMSecs() {
}

void TimerMSecs::SetTime(uint32_t delay_time_ms) {
    if (delay_time_ms < TimerMSecs::min_delay_time_ms) {
        delay_time_ms = TimerMSecs::min_delay_time_ms;
    }
    if (delay_time_ms > TimerMSecs::max_delay_time_ms) {
        delay_time_ms = TimerMSecs::max_delay_time_ms;
    }
    this->delay_time_us = delay_time_ms * 1000LL;
    str_size = sprintf(this->str_time, "%u", delay_time_ms);

    ESP_LOGD(TAG_TimerMSecs, "ctor, str_time:%s", this->str_time);
}

const Bitmap *TimerMSecs::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &TimerMSecs::bitmap_active;

        default:
            return &TimerMSecs::bitmap_passive;
    }
}

size_t TimerMSecs::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&delay_time_us, sizeof(delay_time_us), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t TimerMSecs::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint64_t _delay_time_us;
    if (!Record::Read(&_delay_time_us, sizeof(_delay_time_us), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (_delay_time_us < TimerMSecs::min_delay_time_ms * 1000000LL) {
        return 0;
    }
    if (_delay_time_us > TimerMSecs::max_delay_time_ms * 1000000LL) {
        return 0;
    }
    delay_time_us = _delay_time_us;
    return readed;
}

TvElementType TimerMSecs::GetElementType() {
    return TvElementType::et_TimerMSecs;
}

TimerMSecs *TimerMSecs::TryToCast(CommonTimer *common_timer) {
    switch (common_timer->GetElementType()) {
        case TvElementType::et_TimerMSecs:
            return static_cast<TimerMSecs *>(common_timer);

        default:
            return NULL;
    }
}