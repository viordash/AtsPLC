#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

class button {
  private:
    const static uint32_t pressed_time = 30 / portTICK_PERIOD_MS;
    const static uint32_t long_pressed_time = 3000 / portTICK_PERIOD_MS;

    TickType_t down_time;
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
    EventBits_t pressed_bit;
    EventBits_t long_pressed_bit;

    button(const char *tag,
           EventBits_t close_bit,
           EventBits_t open_bit,
           EventBits_t pressed_bit,
           EventBits_t long_pressed_bit);

    state handle(EventBits_t bits);
};