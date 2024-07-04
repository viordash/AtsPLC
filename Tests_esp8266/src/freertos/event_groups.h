#pragma once

#include "FreeRTOS.h"

typedef void *EventGroupHandle_t;
typedef TickType_t EventBits_t;

EventGroupHandle_t xEventGroupCreate(void);

BaseType_t xEventGroupSetBitsFromISR(EventGroupHandle_t xEventGroup,
                                     const EventBits_t uxBitsToSet,
                                     BaseType_t *pxHigherPriorityTaskWoken);
