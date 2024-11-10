#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LogicProgram/ProcessWakeupService.h"
#include "esp_event.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ProcessWakeupService = "ProcessWakeupService";

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

    requests.insert({ id, next_tick });
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

    for (auto it = requests.begin(); it != requests.end(); it++) {
        auto req = *it;

        if (req.id == id) {
            requests.erase(it);
            break;
        }
    }

    ESP_LOGD(TAG_ProcessWakeupService,
             "RemoveRequest: %p, size:%u, systick:%u",
             id,
             (uint32_t)std::distance(requests.begin(), requests.end()),
             (uint32_t)xTaskGetTickCount());
}

static char *println(std::set<ProcessWakeupRequestData, ProcessWakeupRequestDataCmp> &requests) {
    static char buffer[512];
    int pos = sprintf(buffer, "[");
    bool first{ true };
    for (auto x : requests) {
        pos += sprintf(&buffer[pos], "%s", (first ? first = false, "" : ", "));
        pos += sprintf(&buffer[pos], "%p|%u", x.id, x.next_tick);
    }
    pos += sprintf(&buffer[pos], "]");
    return buffer;
}

uint32_t ProcessWakeupService::Get() {
    if (requests.empty()) {
        ESP_LOGD(TAG_ProcessWakeupService, "Get def:%d", default_delay);
        return default_delay;
    }

    auto current_tick = (uint32_t)xTaskGetTickCount();
    auto req_it = requests.begin();
    auto req = *req_it;
    int timespan = req.next_tick - current_tick;

    ESP_LOGD(TAG_ProcessWakeupService,
             "Get:%d, %p, size:%u, systick:%u, %s",
             timespan,
             req.id,
             (uint32_t)std::distance(requests.begin(), requests.end()),
             current_tick,
             println(requests));
    if (timespan < 0) {
        return 0;
    }
    return (uint32_t)timespan;
}

int ProcessWakeupService::RemoveExpired() {
    auto current_tick = (uint32_t)xTaskGetTickCount();
    int timespan = default_delay;
    while (!requests.empty()) {
        auto req_it = requests.begin();
        auto req = *req_it;
        timespan = req.next_tick - current_tick;
        bool expired = timespan <= 0;
        if (expired) {
            ids.erase(req.id);
            requests.erase(req_it);
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