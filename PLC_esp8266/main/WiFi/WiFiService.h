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
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>

class WiFiService {
  public:
  protected:
    WiFiRequests requests;

    std::mutex scanned_ssid_lock_mutex;
    std::unordered_map<const char *, uint8_t> scanned_ssid;

    uint8_t station_rssi;

    TaskHandle_t task_handle;

    std::mutex station_connect_status_lock_mutex;

    void Connect(wifi_config_t *wifi_config);
    void Disconnect();
    void StationTask(RequestItem *request);
    void ObtainStationRssi();

    bool StartScan(const char *ssid, CurrentSettings::wifi_scanner_settings *scanner_settings);
    int8_t Scanning(RequestItem *request, CurrentSettings::wifi_scanner_settings *scanner_settings);
    void StopScan();
    void ScannerTask(RequestItem *request);
    void AccessPointTask(RequestItem *request);
    static void ap_wifi_event_handler(void *arg,
                                      esp_event_base_t event_base,
                                      int32_t event_id,
                                      void *event_data);

    static void Task(void *parm);
    static void
    wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void
    ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    uint8_t ScaleRssiToPercent04(int8_t rssi, int8_t max_rssi, int8_t min_rssi);
    void AddSsidToScannedList(const char *ssid, uint8_t rssi);
    bool FindSsidInScannedList(const char *ssid, uint8_t *rssi);
    void RemoveSsidFromScannedList(const char *ssid);

  public:
    static const int STOP_BIT = BIT0;
    static const int FAILED_BIT = BIT1;
    static const int CONNECTED_BIT = BIT2;
    static const int CANCEL_REQUEST_BIT = BIT3;

    WiFiService();
    ~WiFiService();

    void Start();
    void Stop();
    bool Started();

    uint8_t ConnectToStation();
    void DisconnectFromStation();

    uint8_t Scan(const char *ssid);
    void CancelScan(const char *ssid);

    void Generate(const char *ssid);
    void CancelGenerate(const char *ssid);
};
