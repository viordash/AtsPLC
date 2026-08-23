#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef enum { //
    NOTHING_PRESSED,
    UP_PRESSED,
    UP_LONG_PRESSED,
    DOWN_PRESSED,
    DOWN_LONG_PRESSED,
    SELECT_PRESSED,
    SELECT_LONG_PRESSED
} ButtonsPressType;

class ProcessWakeupService;

ButtonsPressType handle_buttons(EventBits_t uxBits, ProcessWakeupService *wakeup_service);

ButtonsPressType handle_buttons_immediately(EventBits_t uxBits);
