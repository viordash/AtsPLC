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
  protected:
    enum RequestItemType { //
        wqi_Client = 0,
        wqi_ScanSsid,
        wqi_GenerateSsid
    };

    typedef struct {
        RequestItemType type;
        const char *ssid;
        bool status;
    } RequestItem;

    struct RequestItemComparator {
        bool operator()(const RequestItem &a, const RequestItem &b) const {
            return a.type == b.type && a.ssid == b.ssid;
        }
    };

    struct RequestItemHash {
        size_t operator()(const RequestItem &e1) const {
            return std::hash<size_t>()(((size_t)e1.ssid * 10) + (size_t)e1.type);
        };
    };

    std::unordered_set<RequestItem, RequestItemHash, RequestItemComparator> requests;
    std::mutex lock_mutex;

  public:
    WiFiService();
    ~WiFiService();

    bool Scan(const char *ssid);
    void Generate(const char *ssid);
};
