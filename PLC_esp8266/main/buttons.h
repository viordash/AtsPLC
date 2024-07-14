#pragma once

#include "freertos/event_groups.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define BUTTON_UP_PRESSED BIT0
#define BUTTON_UP_LONG_PRESSED BIT1
#define BUTTON_DOWN_PRESSED BIT2
#define BUTTON_DOWN_LONG_PRESSED BIT3
#define BUTTON_RIGHT_PRESSED BIT4
#define BUTTON_RIGHT_LONG_PRESSED BIT5
#define BUTTON_SELECT_PRESSED BIT6
#define BUTTON_SELECT_LONG_PRESSED BIT7
#define BUTTON_LEFT_PRESSED BIT8

#ifdef __cplusplus
extern "C" {
#endif
EventGroupHandle_t buttons_init(EventGroupHandle_t gpio_events, bool is_hotstart);

#ifdef __cplusplus
}
#endif