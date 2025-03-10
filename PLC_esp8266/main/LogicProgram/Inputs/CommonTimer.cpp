#include "LogicProgram/Inputs/CommonTimer.h"
#include "LogicProgram/Controller.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *TAG_CommonTimer = "CommonTimer";

CommonTimer::CommonTimer() : LogicElement() {
    this->delay_time_us = 0;
}

CommonTimer::~CommonTimer() {
}

bool CommonTimer::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        Controller::RemoveRequestWakeupMs(this);
        return false;
    }
    if (prev_elem_changed && prev_elem_state == LogicItemState::lisActive) {
        Controller::RemoveRequestWakeupMs(this);
        Controller::RequestWakeupMs(this,
                                    delay_time_us / 1000LL,
                                    ProcessWakeupRequestPriority::pwrp_Normal);
    }

    bool any_changes = false;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    if (prev_elem_state != LogicItemState::lisActive) {
        state = LogicItemState::lisPassive;
    } else if (state != LogicItemState::lisActive) {
        bool timer_completed =
            Controller::RequestWakeupMs(this,
                                        delay_time_us / 1000LL,
                                        ProcessWakeupRequestPriority::pwrp_Normal);
        if (timer_completed) {
            state = LogicItemState::lisActive;
        }
    }

    if (state != prev_state) {
        any_changes = true;
        ESP_LOGD(TAG_CommonTimer, ".");
    }
    return any_changes;
}

IRAM_ATTR bool
CommonTimer::Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) {
    bool res = true;
    std::lock_guard<std::recursive_mutex> lock(lock_mutex);

    auto bitmap = GetCurrentBitmap(state);

    if (prev_elem_state == LogicItemState::lisActive) {
        res = draw_active_network(fb, start_point->x, start_point->y, LeftPadding);
    } else {
        res = draw_passive_network(fb, start_point->x, start_point->y, LeftPadding, false);
    }
    if (!res) {
        return res;
    }

    start_point->x += LeftPadding;

    bool blink_bitmap_on_editing = editable_state == EditableElement::ElementState::des_Editing
                                && (CommonTimer::EditingPropertyId)editing_property_id
                                       == CommonTimer::EditingPropertyId::ctepi_None
                                && Blinking_50();
    if (!blink_bitmap_on_editing) {
        draw_bitmap(fb, start_point->x, start_point->y - (bitmap->size.height / 2) + 1, bitmap);
    }

    bool blink_value_on_editing = editable_state == EditableElement::ElementState::des_Editing
                               && (CommonTimer::EditingPropertyId)editing_property_id
                                      == CommonTimer::EditingPropertyId::ctepi_ConfigureDelayTime
                               && Blinking_50();

    switch (str_size) {
        case 1:
            res = blink_value_on_editing
               || (draw_text_f5X7(fb, start_point->x + 10, start_point->y + 2, str_time) > 0);
            break;
        case 2:
            res = blink_value_on_editing
               || (draw_text_f5X7(fb, start_point->x + 6, start_point->y + 2, str_time) > 0);
            break;
        case 3:
            res = blink_value_on_editing
               || (draw_text_f5X7(fb, start_point->x + 3, start_point->y + 2, str_time) > 0);
            break;
        case 4:
            res = blink_value_on_editing
               || (draw_text_f4X7(fb, start_point->x + 4, start_point->y + 3, str_time) > 0);
            break;
        default:
            res = blink_value_on_editing
               || (draw_text_f4X7(fb, start_point->x + 2, start_point->y + 3, str_time) > 0);
            break;
    }

    start_point->x += bitmap->size.width;

    if (res) {
        res = EditableElement::Render(fb, start_point);
    }

    ESP_LOGD(TAG_CommonTimer,
             "Render, str_time:%s, str_size:%d, x:%u, y:%u, res:%u",
             str_time,
             str_size,
             start_point->x,
             start_point->y,
             res);
    return res;
}

CommonTimer *CommonTimer::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_TimerMSecs:
        case TvElementType::et_TimerSecs:
            return static_cast<CommonTimer *>(logic_element);

        default:
            return NULL;
    }
}