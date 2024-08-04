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

void TimerBase::Render(uint8_t *fb) {
    InputBase::Render(fb);

    uint8_t x_pos = incoming_point.x + LeftPadding + LabeledLogicItem::width + 2;

    switch (str_size) {
        case 1:
            draw_text_f5X7(x_pos + 8, incoming_point.y + 2, str_time);
            break;
        case 2:
            draw_text_f5X7(x_pos + 4, incoming_point.y + 2, str_time);
            break;
        case 3:
            draw_text_f5X7(x_pos + 1, incoming_point.y + 2, str_time);
            break;
        case 4:
            draw_text_f4X7(x_pos + 2, incoming_point.y + 3, str_time);
            break;
        default:
            draw_text_f4X7(x_pos, incoming_point.y + 3, str_time);
            break;
    }

    ESP_LOGI(TAG_TimerBase, "Render, str_time:%s, str_size:%d", str_time, str_size);
}