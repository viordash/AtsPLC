#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LogicProgram/ProcessTicksService.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ProcessTicksService = "ProcessTicksService";

int64_t ProcessTicksService::GetTimespan(uint64_t from, uint64_t to) {
    uint64_t timespan = to - from;
    return (int64_t)timespan;
}

int64_t ProcessTicksService::ConvertToSysTickCount(int64_t timespan) {
    uint32_t timespan_ms = timespan / 1000L;
    uint32_t ticks = (timespan_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS;
    return ticks;
}

void ProcessTicksService::Request(uint32_t delay_ms) {
    ESP_LOGI(TAG_ProcessTicksService,
             "Request:%u, size:%u",
             delay_ms,
             (uint32_t)std::distance(moments.begin(), moments.end()));

    auto curr_time = (uint64_t)esp_timer_get_time();
    auto next_moment = curr_time + (delay_ms * 1000LL);
    auto next_sys_ticks = ConvertToSysTickCount(next_moment);

    auto it = moments.begin();
    auto it_prev = moments.before_begin();
    while (it != moments.end()) {
        auto moment = *it;

        int64_t timespan_from_current = GetTimespan(curr_time, moment);
        bool expired = timespan_from_current < 0;
        if (expired) {
            it = moments.erase_after(it_prev);
            continue;
        }

        bool filter_unique = next_sys_ticks == ConvertToSysTickCount(moment);
        if (filter_unique) {
            return;
        }

        int timespan = GetTimespan(next_moment, moment);
        bool further_large_values = timespan > 0;
        if (further_large_values) {
            break;
        }
        it_prev = it;
        it++;
    }
    moments.insert_after(it_prev, next_moment);
}

uint32_t ProcessTicksService::Get() {
    if (!moments.empty()) {
        auto curr_time = esp_timer_get_time();
        int64_t timespan;
        do {
            auto next_moment = moments.front();
            moments.pop_front();
            timespan = next_moment - curr_time;
        } while (!moments.empty() && timespan < 0);

        if (timespan >= 0) {
            auto ticks = ConvertToSysTickCount(timespan);
            ESP_LOGI(TAG_ProcessTicksService,
                     "Get:%d(%d), size:%u",
                     (int)timespan, (int)ticks,
                     (uint32_t)std::distance(moments.begin(), moments.end()));
            return ticks;
        }
        return 0;
    }
    ESP_LOGI(TAG_ProcessTicksService, "Get def:%d", default_delay);
    return default_delay;
}
