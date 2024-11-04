#include "LogicProgram/ProcessTicksService.h"
#include "esp_event.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ProcessTicksService::ProcessTicksService(/* args */) {
}

ProcessTicksService::~ProcessTicksService() {
}

void ProcessTicksService::Request(uint32_t delay_ms) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto current_tick = xTaskGetTickCount();
    auto next_tick = current_tick + (delay_ms / portTICK_PERIOD_MS);

    auto it = ticks.begin();
    auto it_prev = ticks.before_begin();
    while (it != ticks.end()) {
        auto tick = *it;
        int timespan_from_current = tick - current_tick;

        bool expired = timespan_from_current < 0;
        if (expired) {
            it = ticks.erase_after(it_prev);
            continue;
        }

        bool filter_unique = tick == next_tick;
        if (filter_unique) {
            return;
        }

        int timespan = tick - next_tick;
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
        auto current_tick = xTaskGetTickCount();
        int timespan;
        do {
            auto next_tick = ticks.front();
            ticks.pop_front();
            timespan = next_tick - current_tick;
        } while (!ticks.empty() && timespan < 0);

        if (timespan >= 0) {
            return (TickType_t)timespan;
        }
    }
    return default_delay;
}
