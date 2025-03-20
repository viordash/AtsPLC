#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "buttons.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

class button {
  private:
    const static uint32_t pressed_time_ms = 20;
    const static uint32_t long_pressed_time_ms = 500;

    uint64_t down_time;
    EventBits_t close_bit;
    EventBits_t open_bit;
    bool down;

  public:
    enum state {
        btNone = 0,
        btDown = 0x01,
        btShortPressed = 0x02,
        btPressed = 0x04,
        btLongPressed = 0x08,
    };
    const char *TAG;
    ButtonsPressType pressed_type;
    ButtonsPressType long_pressed_type;

    button(const char *tag,
           EventBits_t close_bit,
           EventBits_t open_bit,
           ButtonsPressType pressed_type,
           ButtonsPressType long_pressed_type);

    state handle(EventBits_t bits);
};
