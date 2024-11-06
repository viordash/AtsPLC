#include "LogicProgram/Inputs/TimerSecs.h"
#include "Display/bitmaps/timer_sec_active.h"
#include "Display/bitmaps/timer_sec_passive.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/Serializer/Record.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
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
    str_size = sprintf(this->str_time, "%u", delay_time_s);

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

bool TimerSecs::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    bool any_changes = CommonTimer::DoAction(prev_elem_changed, prev_elem_state);

    if (prev_elem_state == LogicItemState::lisActive
        && (any_changes || state != LogicItemState::lisActive)) {
        Controller::RequestWakeupMs(((delay_time_us / 4) + 1000LL - 1) / 1000LL);
    }

    if (!any_changes) {
        any_changes = ProgressHasChanges(prev_elem_state);
    }
    return any_changes;
}

IRAM_ATTR bool TimerSecs::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    uint8_t x_pos = start_point->x + LeftPadding - VERT_PROGRESS_BAR_WIDTH;

    res = CommonTimer::Render(fb, prev_elem_state, start_point);

    if (prev_elem_state == LogicItemState::lisActive) {
        uint8_t percent = GetProgress(prev_elem_state);
        res = draw_vert_progress_bar(fb,
                                     x_pos,
                                     start_point->y - (VERT_PROGRESS_BAR_HEIGHT + 1),
                                     percent);
        ESP_LOGD(TAG_TimerSecs,
                 "Render, percent:%u, delay:%u",
                 percent,
                 (uint32_t)(delay_time_us / 1000000LL));
    }

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