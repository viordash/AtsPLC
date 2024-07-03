#pragma once

#include "freertos/event_groups.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif
EventGroupHandle_t buttons_init(EventGroupHandle_t gpio_events);

#ifdef __cplusplus
}
#endif