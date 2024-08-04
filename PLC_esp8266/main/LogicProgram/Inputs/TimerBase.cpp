#include "LogicProgram/Inputs/TimerBase.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

TimerBase::TimerBase(uint64_t time_us, InputBase &prior_item)
    : InputBase(MapIO::Undef, prior_item) {
    this->raise_time_us = time_us + (uint64_t)esp_timer_get_time();
}

TimerBase::~TimerBase() {
}

void TimerBase::Render(uint8_t *fb) {
    InputBase::Render(fb);

    uint8_t x_pos = incoming_point.x + LeftPadding + LabeledLogicItem::width + 2;
    if (text_f5X7) {
        draw_text_f5X7(x_pos, incoming_point.y + 2, str_time);
    } else {
        draw_text_f4X7(x_pos, incoming_point.y + 3, str_time);
    }
}