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

#include "WiFiRequests.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_set>

class WiFiService {
  public:
  protected:
    WiFiRequests requests;

    std::mutex scanned_ssid_lock_mutex;
    std::unordered_set<const char *> scanned_ssid;

    EventGroupHandle_t event;

    void Connect(wifi_config_t *wifi_config);
    void Disconnect();
    bool ConnectToStationTask(wifi_config_t *wifi_config,
                              bool *has_new_request,
                              int32_t max_retry_count);
    bool StationTask();
    bool ScannerTask(RequestItem *request);
    bool AccessPointTask(RequestItem *request);

    static void Task(void *parm);
    static void
    wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void
    ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    void AddSsidToScannedList(const char *ssid);
    bool FindSsidInScannedList(const char *ssid);
    void RemoveSsidFromScannedList(const char *ssid);

  public:
    static const int STARTED_BIT = BIT0;
    static const int RUNNED_BIT = BIT1;
    static const int STOP_BIT = BIT2;
    static const int STOPPED_BIT = BIT3;
    static const int FAILED_BIT = BIT4;
    static const int CONNECTED_BIT = BIT5;
    static const int NEW_REQUEST_BIT = BIT6;
    static const int CANCEL_REQUEST_BIT = BIT7;

    WiFiService();
    ~WiFiService();

    void Start();
    void Stop();
    bool Started();

    void ConnectToStation();
    bool Scan(const char *ssid);
    void CancelScan(const char *ssid);

    void Generate(const char *ssid);
    void CancelGenerate(const char *ssid);
};
