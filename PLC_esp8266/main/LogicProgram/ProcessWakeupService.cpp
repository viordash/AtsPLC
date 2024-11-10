#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LogicProgram/ProcessWakeupService.h"
#include "esp_event.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ProcessWakeupService = "ProcessWakeupService";

int32_t ProcessWakeupService::GetTimespan(uint32_t from, uint32_t to) {
    uint32_t timespan = to - from;
    return (int32_t)timespan;
}

void ProcessWakeupService::Request(void *id, uint32_t delay_ms) {
    bool request_already_in = ids.find(id) != ids.end();
    if (request_already_in) {
        ESP_LOGD(TAG_ProcessWakeupService,
                 "Request already in:%u, %p, size:%u",
                 ((delay_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS),
                 id,
                 (uint32_t)std::distance(requests.begin(), requests.end()));
        return;
    }

    auto current_tick = (uint32_t)xTaskGetTickCount();
    auto next_tick = current_tick + ((delay_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS);

    auto it = requests.begin();
    auto it_prev = requests.before_begin();
    while (it != requests.end()) {
        auto req = *it;

        int32_t timespan_from_current = GetTimespan(current_tick, req.next_tick);
        bool expired = timespan_from_current < 0;
        if (expired) {
            it = requests.erase_after(it_prev);
            continue;
        }

        bool filter_unique = req.next_tick == next_tick;
        if (filter_unique) {
            return;
        }

        int timespan = GetTimespan(next_tick, req.next_tick);
        bool further_large_values = timespan > 0;
        if (further_large_values) {
            break;
        }
        it_prev = it;
        it++;
    }
    requests.insert_after(it_prev, { id, next_tick });
    ids.insert(id);

    ESP_LOGD(TAG_ProcessWakeupService,
             "Request:%u, %p, size:%u, systick:%u",
             ((delay_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS),
             id,
             (uint32_t)std::distance(requests.begin(), requests.end()),
             current_tick);
}

void ProcessWakeupService::RemoveRequest(void *id) {
    auto id_it = ids.find(id);
    bool request_exists = id_it != ids.end();
    if (!request_exists) {
        return;
    }
    ids.erase(id_it);

    auto it = requests.begin();
    auto it_prev = requests.before_begin();
    while (it != requests.end()) {
        auto req = *it;

        if (req.id == id) {
            requests.erase_after(it_prev);
            break;
        }
        it_prev = it;
        it++;
    }

    ESP_LOGD(TAG_ProcessWakeupService,
             "RemoveRequest: %p, size:%u, systick:%u",
             id,
             (uint32_t)std::distance(requests.begin(), requests.end()),
             (uint32_t)xTaskGetTickCount());
}

uint32_t ProcessWakeupService::Get() {
    if (requests.empty()) {
        ESP_LOGD(TAG_ProcessWakeupService, "Get def:%d", default_delay);
        return default_delay;
    }

    auto current_tick = (uint32_t)xTaskGetTickCount();
    auto req = requests.front();
    int timespan = req.next_tick - current_tick;

    ESP_LOGD(TAG_ProcessWakeupService,
             "Get:%d, %p, size:%u, systick:%u",
             timespan,
             req.id,
             (uint32_t)std::distance(requests.begin(), requests.end()),
             current_tick);
    if (timespan < 0) {
        return 0;
    }
    return (uint32_t)timespan;
}

int ProcessWakeupService::RemoveExpired() {
    auto current_tick = (uint32_t)xTaskGetTickCount();
    int timespan = default_delay;
    while (!requests.empty()) {
        auto req = requests.front();
        timespan = req.next_tick - current_tick;
        bool expired = timespan <= 0;
        if (expired) {
            ids.erase(req.id);
            requests.pop_front();
            ESP_LOGD(TAG_ProcessWakeupService,
                     "RemoveExpired: %p, size:%u, systick:%u",
                     req.id,
                     (uint32_t)std::distance(requests.begin(), requests.end()),
                     current_tick);
        } else {
            break;
        }
    }
    return timespan;
}