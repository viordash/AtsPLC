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

#include "ApClients.h"
#include "ScannedSsid.h"
#include "WiFiRequests.h"
#include "config.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi_types.h"
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class WiFiService {
  public:
    struct AccessPointEventArg {
        WiFiService *service;
        const char *ssid;
        const char *mac;
    };

  protected:
    WiFiRequests requests;

    std::mutex scanned_ssid_lock_mutex;
    ScannedSsid scanned_ssids[WiFi_SsidLimit];
    size_t scanned_ssids_count;

    uint8_t station_rssi;

    std::mutex ap_clients_lock_mutex;
    ApClients ap_clients[WiFi_SsidLimit];
    size_t ap_clients_count;

    TaskHandle_t task_handle;

    std::mutex station_connect_status_lock_mutex;

    void Connect(wifi_config_t *wifi_config);
    void Disconnect();
    void StationTask(RequestItem *request);
    bool ObtainStationRssi();

    bool StartScan(const char *ssid, CurrentSettings::wifi_scanner_settings *scanner_settings);
    int8_t Scanning(RequestItem *request,
                    CurrentSettings::wifi_scanner_settings *scanner_settings,
                    bool *canceled);
    void StopScan();
    void ScannerTask(RequestItem *request);
    void AccessPointTask(RequestItem *request);
    static void ap_connect_wifi_event_handler(void *arg,
                                              esp_event_base_t event_base,
                                              int32_t event_id,
                                              void *event_data);
    static void ap_disconnect_wifi_event_handler(void *arg,
                                                 esp_event_base_t event_base,
                                                 int32_t event_id,
                                                 void *event_data);

    static void Task(void *parm);
    static void
    wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void
    ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    uint8_t ScaleRssiToPercent04(int8_t rssi, int8_t max_rssi, int8_t min_rssi);
    ScannedSsid *FindScanned(const char *ssid);
    ApClients *FindApClients(const char *ssid);
    void AddScannedSsid(const char *ssid, uint8_t rssi);
    bool FindScannedSsid(const char *ssid, uint8_t *rssi);
    void RemoveScannedSsid(const char *ssid);

    void AddApClient(const char *ssid, t_mac mac);
    size_t GetApClientsCount(const char *ssid);
    void RemoveApClient(const char *ssid, t_mac mac);
    void RemoveApClients(const char *ssid);

  public:
    static const int STA_FAILED_BIT = BIT1;
    static const int STA_CONNECTED_BIT = BIT2;
    static const int STA_BREAK_BIT = BIT3;
    static const int SCAN_BREAK_BIT = BIT4;
    static const int AP_BREAK_BIT = BIT5;

    WiFiService();
    ~WiFiService();

    void Start();

    uint8_t ConnectToStation();
    void DisconnectFromStation();

    uint8_t Scan(const char *ssid);
    void CancelScan(const char *ssid);

    size_t AccessPoint(const char *ssid, const char *password, const char *mac);
    void CancelAccessPoint(const char *ssid);
};
