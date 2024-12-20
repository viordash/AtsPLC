#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

#include "esp_err.h"
#include "esp_log.h"
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_set>

class WiFiService {
    enum RequestItemType { //
        wqi_Client = 0,
        wqi_ScanSsid,
        wqi_GenerateSsid
    };

    typedef struct {
        RequestItemType type;
        const char *ssid;
    } RequestItem;

    struct RequestItemEqual {
        bool operator()(const RequestItem &a, const RequestItem &b) {
            return a.type == b.type && strcmp(a.ssid, b.ssid) == 0;
        }
    };

  protected:
    static std::unordered_set<RequestItem, RequestItemEqual> requests;
    static std::mutex lock_mutex;

  public:
    static void Start();
    static void Stop();

    static bool Find(const char *ssid);
    static bool Generate(const char *ssid);
};
