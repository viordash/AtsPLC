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
    uint32_t delay_ticks = delay_ms / portTICK_PERIOD_MS;
    auto it = delays.begin();
    if (delays.empty() || *it > delay_ticks) {
        delays.push_front(delay_ticks);
        return;
    }

    auto it_prev = it;
    while (it != delays.end()) {
        auto delay = *it;
        bool filter_unique = delay == delay_ticks;
        if (filter_unique) {
            return;
        }

        bool further_large_values = delay > delay_ticks;
        if (further_large_values) {
            break;
        }
        it_prev = it;
        it++;
    }
    delays.insert_after(it_prev, delay_ticks);
}

uint32_t ProcessTicksService::PopTicksToWait() {
    if (delays.empty()) {
        return default_delay;
    }
    auto smallest_delay = delays.front();
    delays.pop_front();

    for (auto it = delays.begin(); it != delays.end(); ++it) {
        auto delay = *it;
        *it = delay - smallest_delay;
    }

    return smallest_delay;
}
