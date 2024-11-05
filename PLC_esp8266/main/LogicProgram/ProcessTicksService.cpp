#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LogicProgram/ProcessTicksService.h"
#include "esp_event.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ProcessTicksService = "ProcessTicksService";

int32_t ProcessTicksService::GetTimespan(uint32_t from, uint32_t to) {
    uint32_t timespan = to - from;
    return (int32_t)timespan;
}

void ProcessTicksService::Request(uint32_t delay_ms) {
    ESP_LOGD(TAG_ProcessTicksService, "Request:%u", delay_ms);

    auto current_tick = (uint32_t)xTaskGetTickCount();
    auto next_tick = current_tick + (delay_ms / portTICK_PERIOD_MS);
    std::lock_guard<std::mutex> lock(lock_mutex);

    auto it = ticks.begin();
    auto it_prev = ticks.before_begin();
    while (it != ticks.end()) {
        auto tick = *it;

        int32_t timespan_from_current = GetTimespan(current_tick, tick);
        bool expired = timespan_from_current < 0;
        if (expired) {
            it = ticks.erase_after(it_prev);
            continue;
        }

        bool filter_unique = tick == next_tick;
        if (filter_unique) {
            return;
        }

        int timespan = GetTimespan(next_tick, tick);
        bool further_large_values = timespan > 0;
        if (further_large_values) {
            break;
        }
        it_prev = it;
        it++;
    }
    ticks.insert_after(it_prev, next_tick);
}

TickType_t ProcessTicksService::Get() {
    if (!ticks.empty()) {
        std::lock_guard<std::mutex> lock(lock_mutex);
        auto current_tick = (uint32_t)xTaskGetTickCount();
        int timespan;
        do {
            auto next_tick = ticks.front();
            ticks.pop_front();
            timespan = next_tick - current_tick;
        } while (!ticks.empty() && timespan < 0);

        if (timespan >= 0) {
            ESP_LOGI(TAG_ProcessTicksService, "Get:%u", (TickType_t)timespan);
            return (TickType_t)timespan;
        }
    }
    ESP_LOGI(TAG_ProcessTicksService, "Get def:%u", default_delay);
    return default_delay;
}
