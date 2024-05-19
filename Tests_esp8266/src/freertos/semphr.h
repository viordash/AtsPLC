#pragma once

typedef void *SemaphoreHandle_t;

#define xSemaphoreCreateMutex() ((SemaphoreHandle_t)1)

#define xSemaphoreTake(xSemaphore, xBlockTime) (pdTRUE)

#define xSemaphoreGive(xSemaphore) (pdTRUE)