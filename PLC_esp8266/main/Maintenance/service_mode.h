#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

void run_service_mode(EventGroupHandle_t gpio_events);

#ifdef __cplusplus
}
#endif