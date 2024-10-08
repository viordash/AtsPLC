#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

void start_process_engine(EventGroupHandle_t gpio_events);
void stop_process_engine();

#ifdef __cplusplus
}
#endif