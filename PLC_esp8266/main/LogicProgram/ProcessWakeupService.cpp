#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LogicProgram/ProcessWakeupService.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ProcessWakeupService = "ProcessWakeupService";

bool ProcessWakeupService::Request(void *id,
                                   uint32_t delay_ms,
                                   ProcessWakeupRequestPriority priority) {
    bool request_already_in = ids.find(id) != ids.end();
    if (request_already_in) {
        ESP_LOGD(TAG_ProcessWakeupService,
                 "Request already in:%u, %p, size:%u",
                 (unsigned int)delay_ms,
                 id,
                 (unsigned int)std::distance(requests.begin(), requests.end()));
        return false;
    }

    auto current_time = (uint64_t)esp_timer_get_time();
    auto next_time = current_time + ((uint64_t)delay_ms * 1000);

    ProcessWakeupRequestData request = { id, next_time, priority };
    auto upper = requests.upper_bound(request);
    if (upper != requests.end()) {
        auto upper_req = *upper;
        bool request_can_be_joined = request.priority == ProcessWakeupRequestPriority::pwrp_Idle
                                  && (upper_req.next_time - next_time) < idle_dead_band_us;
        if (request_can_be_joined) {
            ESP_LOGD(TAG_ProcessWakeupService,
                     "Request is joined in:%u, %p, diff:%d, next:%u",
                     (unsigned int)delay_ms,
                     id,
                     (int)(upper_req.next_time - next_time),
                     (unsigned int)upper_req.next_time);
            request.next_time = upper_req.next_time;
        }
    }

    requests.insert(request);
    ids.insert(id);

    ESP_LOGD(TAG_ProcessWakeupService,
             "Request:%u, %p, size:%u, time:%u",
             (unsigned int)delay_ms,
             id,
             (unsigned int)std::distance(requests.begin(), requests.end()),
             (unsigned int)(current_time / 1000));
    return true;
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
             (unsigned int)std::distance(requests.begin(), requests.end()),
             (unsigned int)esp_timer_get_time());
}

static char *
println(const std::set<ProcessWakeupRequestData, ProcessWakeupRequestDataCmp> &requests) {
    static char buffer[512];
    int pos = sprintf(buffer, "[");
    bool first{ true };
    for (auto x : requests) {
        pos += sprintf(&buffer[pos], "%s", (first ? first = false, "" : ", "));
        pos += sprintf(&buffer[pos], "%p|%u", x.id, (unsigned int)(x.next_time / 1000));
    }
    printf(&buffer[pos], "]");
    return buffer;
}

uint32_t ProcessWakeupService::Get() {
    if (requests.empty()) {
        ESP_LOGD(TAG_ProcessWakeupService, "Get def:%u", (unsigned int)default_delay);
        return default_delay;
    }

    auto current_time = (uint64_t)esp_timer_get_time();
    auto req_it = requests.begin();
    auto req = *req_it;
    int64_t timespan = req.next_time - current_time;

    uint32_t wait_ticks =
        ((timespan / 1000) + (portTICK_PERIOD_MS - portTICK_PERIOD_MS / 2)) / portTICK_PERIOD_MS;

    ESP_LOGD(TAG_ProcessWakeupService,
             "Get:%u, %p, size:%u, time:%u, %s",
             (unsigned int)wait_ticks,
             req.id,
             (unsigned int)std::distance(requests.begin(), requests.end()),
             (unsigned int)(current_time / 1000),
             println(requests));
    if (timespan < 0) {
        return 0;
    }
    return wait_ticks;
}

int ProcessWakeupService::RemoveExpired() {
    auto current_time = (uint64_t)esp_timer_get_time();
    int64_t timespan = default_delay;
    while (!requests.empty()) {
        auto req_it = requests.begin();
        auto req = *req_it;
        timespan = req.next_time - current_time;
        bool expired = timespan <= (portTICK_PERIOD_MS / 2) * 1000;
        if (expired) {
            ids.erase(req.id);
            requests.erase(req_it);
            ESP_LOGD(TAG_ProcessWakeupService,
                     "RemoveExpired: %p, size:%u, systick:%u",
                     req.id,
                     (unsigned int)std::distance(requests.begin(), requests.end()),
                     (unsigned int)(current_time / 1000));
        } else {
            break;
        }
    }
    return (int)timespan;
}