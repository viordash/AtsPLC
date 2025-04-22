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
#include <unordered_set>

class WiFiService {
  public:
    struct AccessPointEventArg {
        WiFiService *service;
        const char *ssid;
        const char *mac;
    };

    using t_mac = uint64_t;

  protected:
    WiFiRequests requests;

    std::mutex scanned_ssid_lock_mutex;
    std::unordered_map<const char *, uint8_t> scanned_ssid;

    uint8_t station_rssi;

    std::mutex ap_clients_lock_mutex;
    std::unordered_map<const char *, std::unordered_set<t_mac>> ap_clients;

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
    void AddScannedSsid(const char *ssid, uint8_t rssi);
    bool FindScannedSsid(const char *ssid, uint8_t *rssi);
    void RemoveScannedSsid(const char *ssid);

    void AddApClient(const char *ssid, t_mac mac);
    size_t GetApClientsCount(const char *ssid);
    void RemoveApClient(const char *ssid, t_mac mac);
    void RemoveApClients(const char *ssid);

  public:
    static const int STOP_BIT = BIT0;
    static const int FAILED_BIT = BIT1;
    static const int CONNECTED_BIT = BIT2;
    static const int CANCEL_REQUEST_BIT = BIT3;

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
