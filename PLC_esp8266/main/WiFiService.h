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
#include "esp_event.h"
#include "esp_log.h"
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_set>

class WiFiService {
  protected:
    enum RequestItemType { //
        wqi_Connect = 0,
        wqi_ScanSsid,
        wqi_GenerateSsid
    };

    typedef struct {
        RequestItemType type;
        union {
            struct {
                const char *ssid;
                bool status;
            } ScanSsid;
            struct {
                const char *ssid;
            } GenerateSsid;
        } Payload;
    } RequestItem;

    struct RequestItemComparator {
        bool operator()(const RequestItem &a, const RequestItem &b) const {
            if (a.type != b.type) {
                return false;
            }
            switch (a.type) {
                case wqi_ScanSsid:
                    return a.Payload.ScanSsid.ssid == b.Payload.ScanSsid.ssid;

                case wqi_GenerateSsid:
                    return a.Payload.GenerateSsid.ssid == b.Payload.GenerateSsid.ssid;

                default:
                    return true;
            }
        }
    };

    struct RequestItemHash {
        size_t operator()(const RequestItem &e1) const {
            switch (e1.type) {
                case wqi_ScanSsid:
                    return std::hash<size_t>()(((size_t)e1.Payload.ScanSsid.ssid * 10)
                                               + (size_t)e1.type);

                case wqi_GenerateSsid:
                    return std::hash<size_t>()(((size_t)e1.Payload.GenerateSsid.ssid * 10)
                                               + (size_t)e1.type);

                default:
                    return std::hash<size_t>()(e1.type);
            };
        };
    };

    std::unordered_set<RequestItem, RequestItemHash, RequestItemComparator> requests;
    std::mutex lock_mutex;

    static const int STARTED_BIT = BIT0;
    static const int RUNNED_BIT = BIT1;
    static const int STOP_BIT = BIT2;
    static const int STOPPED_BIT = BIT3;
    static const int FAILED_BIT = BIT4;
    static const int CONNECTED_BIT = BIT5;
    static const int NEW_REQUEST_BIT = BIT6;
    EventGroupHandle_t event;

    void Connect();
    void Disconnect();

    static void Task(void *parm);
    static void
    wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void
    ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

  public:
    WiFiService();
    ~WiFiService();

    void Start();
    void Stop();
    bool Started();

    bool Scan(const char *ssid);
    void Generate(const char *ssid);
};
