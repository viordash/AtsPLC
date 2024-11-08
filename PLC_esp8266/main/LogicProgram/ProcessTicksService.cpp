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
    auto current_tick = (uint32_t)xTaskGetTickCount();

    auto next_tick = current_tick + ((delay_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS);

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

    ESP_LOGD(TAG_ProcessTicksService,
             "Request:%u, tick:%u, size:%u, systick:%u",
             ((delay_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS),
             current_tick,
             (uint32_t)std::distance(ticks.begin(), ticks.end()),
             current_tick);
}

uint32_t ProcessTicksService::Get() {
    if (ticks.empty()) {
        ESP_LOGD(TAG_ProcessTicksService, "Get def:%d", default_delay);
        return default_delay;
    }

    auto current_tick = (uint32_t)xTaskGetTickCount();
    auto next_tick = ticks.front();
    int timespan = next_tick - current_tick;

    ESP_LOGD(TAG_ProcessTicksService,
             "Get:%d, size:%u, systick:%u",
             timespan,
             (uint32_t)std::distance(ticks.begin(), ticks.end()),
             current_tick);
    if (timespan < 0) {
        return 0;
    }
    return (uint32_t)timespan;
}

int ProcessTicksService::RemoveExpired() {
    auto current_tick = (uint32_t)xTaskGetTickCount();
    int timespan = default_delay;
    while (!ticks.empty()) {
        auto next_tick = ticks.front();
        timespan = next_tick - current_tick;
        bool expired = timespan <= 0;
        if (expired) {
            ticks.pop_front();
            ESP_LOGD(TAG_ProcessTicksService,
                     "RemoveExpired:%d, size:%u, systick:%u",
                     timespan,
                     (uint32_t)std::distance(ticks.begin(), ticks.end()),
                     current_tick);
        } else {
            break;
        }
    }
    return timespan;
}