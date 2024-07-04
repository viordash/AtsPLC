#pragma once

typedef void * EventGroupHandle_t;

#define xEventGroupSetBitsFromISR( xEventGroup, uxBitsToSet, pxHigherPriorityTaskWoken )