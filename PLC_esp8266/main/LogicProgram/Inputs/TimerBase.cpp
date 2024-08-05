#include "LogicProgram/Inputs/TimerBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *TAG_TimerBase = "TimerBase";

TimerBase::TimerBase(InputBase &prior_item) : InputBase(MapIO::Undef, prior_item) {
}

TimerBase::~TimerBase() {
}

uint64_t TimerBase::GetLeftTime() {
    uint64_t curr_time = esp_timer_get_time();
    uint64_t left_time;
    if (raise_time_us >= curr_time) {
        left_time = raise_time_us - curr_time;
    } else {
        left_time = raise_time_us + 1 + (UINT64_MAX - curr_time);
    }

    if (left_time > delay_time_us) {
        left_time = 0;
    }
    return left_time;
}

uint8_t TimerBase::GetProgress() {
    uint64_t left_time = GetLeftTime();
    uint8_t percent = (left_time * 100) / delay_time_us;
    return 100 - (uint8_t)percent;
}

void TimerBase::Render(uint8_t *fb) {
    InputBase::Render(fb);

    uint8_t x_pos = incoming_point.x + LeftPadding + LabeledLogicItem::width;

    switch (str_size) {
        case 1:
            draw_text_f5X7(x_pos + 10, incoming_point.y + 2, str_time);
            break;
        case 2:
            draw_text_f5X7(x_pos + 6, incoming_point.y + 2, str_time);
            break;
        case 3:
            draw_text_f5X7(x_pos + 3, incoming_point.y + 2, str_time);
            break;
        case 4:
            draw_text_f4X7(x_pos + 4, incoming_point.y + 3, str_time);
            break;
        default:
            draw_text_f4X7(x_pos + 2, incoming_point.y + 3, str_time);
            break;
    }

    x_pos = x_pos - LabeledLogicItem::width / 2;
    uint8_t percent = GetProgress();
    draw_progress_bar(x_pos, incoming_point.y - (PROGRESS_BAR_HEIGHT + 1), percent);

    ESP_LOGD(TAG_TimerBase, "Render, str_time:%s, str_size:%d", str_time, str_size);
}