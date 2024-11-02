#include "LogicProgram/ProcessTicksService.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sys_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ProcessTicksService::ProcessTicksService(/* args */) {
}

ProcessTicksService::~ProcessTicksService() {
}

void ProcessTicksService::Request(uint32_t delay_ms) {
    auto it = delays.begin();
    while (it != delays.end()) {
        auto delay = *it;
        bool further_large_values = delay > delay_ms;
        if (further_large_values) {
            delays.insert(it, delay_ms);
            return;
        }

        int diff = delay_ms - delay;
        bool filter_identical = diff < min_time_step_ms;
        if (filter_identical) {
            return;
        }
        it++;
    }
    delays.push_back(delay_ms);
}

uint32_t ProcessTicksService::PopTicksToWait() {
    if (delays.empty()) {
        return default_delay_ms / portTICK_PERIOD_MS;
    }
    auto smallest_delay = delays.front();
    delays.pop_front();

    for (auto it = delays.begin(); it != delays.end(); ++it) {
        auto delay = *it;
        *it = delay - smallest_delay;
    }

    return smallest_delay / portTICK_PERIOD_MS;
}
