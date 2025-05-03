#include "LogicProgram/Inputs/TimerSecs.h"
#include "Display/bitmaps/timer_sec_active.h"
#include "Display/bitmaps/timer_sec_passive.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *TAG_TimerSecs = "TimerSecs";

TimerSecs::TimerSecs() : CommonTimer() {
}

TimerSecs::TimerSecs(uint32_t delay_time_s) : TimerSecs() {
    SetTime(delay_time_s);
}

TimerSecs::~TimerSecs() {
}

void TimerSecs::SetTime(uint32_t delay_time_s) {
    if (delay_time_s < TimerSecs::min_delay_time_s) {
        delay_time_s = TimerSecs::min_delay_time_s;
    }
    if (delay_time_s > TimerSecs::max_delay_time_s) {
        delay_time_s = TimerSecs::max_delay_time_s;
    }
    this->delay_time_us = delay_time_s * 1000000LL;
    str_size = sprintf(this->str_time, "%u", (unsigned int)delay_time_s);

    ESP_LOGD(TAG_TimerSecs, "ctor, str_time:%s", this->str_time);
}

uint64_t TimerSecs::GetTimeUs() {
    return this->delay_time_us;
}

const Bitmap *TimerSecs::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &TimerSecs::bitmap_active;

        default:
            return &TimerSecs::bitmap_passive;
    }
}

size_t TimerSecs::Serialize(uint8_t *buffer, size_t buffer_size) {
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

size_t TimerSecs::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint64_t _delay_time_us;
    if (!Record::Read(&_delay_time_us, sizeof(_delay_time_us), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (_delay_time_us < TimerSecs::min_delay_time_s * 1000000LL) {
        return 0;
    }
    if (_delay_time_us > TimerSecs::max_delay_time_s * 1000000LL) {
        return 0;
    }
    SetTime(_delay_time_us / 1000000LL);
    return readed;
}

TvElementType TimerSecs::GetElementType() {
    return TvElementType::et_TimerSecs;
}

TimerSecs *TimerSecs::TryToCast(CommonTimer *common_timer) {
    switch (common_timer->GetElementType()) {
        case TvElementType::et_TimerSecs:
            return static_cast<TimerSecs *>(common_timer);

        default:
            return NULL;
    }
}

void TimerSecs::SelectPrior() {
    ESP_LOGI(TAG_TimerSecs, "SelectPrior");
    uint32_t delay_time_s = GetTimeUs() / 1000000LL;
    if (delay_time_s <= TimerSecs::max_delay_time_s - step_s) {
        SetTime(delay_time_s + step_s);
    } else {
        SetTime(TimerSecs::max_delay_time_s);
    }
}

void TimerSecs::SelectNext() {
    ESP_LOGI(TAG_TimerSecs, "SelectNext");

    uint32_t delay_time_s = GetTimeUs() / 1000000LL;
    if (delay_time_s >= TimerSecs::min_delay_time_s + step_s) {
        SetTime(delay_time_s - step_s);
    } else {
        SetTime(TimerSecs::min_delay_time_s);
    }
}

void TimerSecs::PageUp() {
    uint32_t delay_time_s = GetTimeUs() / 1000000LL;
    if (delay_time_s <= TimerSecs::max_delay_time_s - faststep_s) {
        SetTime(delay_time_s + faststep_s);
    } else {
        SetTime(TimerSecs::max_delay_time_s);
    }
}

void TimerSecs::PageDown() {
    uint32_t delay_time_s = GetTimeUs() / 1000000LL;
    if (delay_time_s >= TimerSecs::min_delay_time_s + faststep_s) {
        SetTime(delay_time_s - faststep_s);
    } else {
        SetTime(TimerSecs::min_delay_time_s);
    }
}

void TimerSecs::Change() {
    ESP_LOGI(TAG_TimerSecs, "Change");
    switch (editing_property_id) {
        case TimerSecs::EditingPropertyId::ctepi_None:
            editing_property_id = TimerSecs::EditingPropertyId::ctepi_ConfigureDelayTime;
            break;

        default:
            EndEditing();
            break;
    }
}

void TimerSecs::Option() {
}