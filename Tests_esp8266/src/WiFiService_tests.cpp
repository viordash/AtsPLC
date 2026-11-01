#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "LogicProgram/LogicElement.h"
#include "main/LogicProgram/Controller.h"
#include "main/WiFi/WiFiService.h"
#include "main/settings.h"

static ProcessWakeupService *process_wakeup_service;

TEST_GROUP(WiFiServiceTestsGroup){
    //
    TEST_SETUP(){ mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
mock().expectOneCall("xTaskCreate").ignoreOtherParameters();

settings.wifi_scanner.per_channel_scan_time_ms = 500;
settings.wifi_scanner.max_rssi = -26;
settings.wifi_scanner.min_rssi = -120;

settings.wifi_station.reconnect_delay_ms = 3000;
settings.wifi_station.scan_station_rssi_period_ms = 5000;

process_wakeup_service = new ProcessWakeupService();
Controller::Start(NULL, NULL, NULL, NULL, process_wakeup_service);
}

TEST_TEARDOWN() {
    Controller::Stop();
    delete process_wakeup_service;
}
}
;

namespace {
    class TestableWiFiService : public WiFiService {
      public:
        TestableWiFiService() : WiFiService() {
        }
        virtual ~TestableWiFiService() {
        }

        WiFiRequests *PublicMorozov_Get_requests() {
            return &requests;
        }
        bool PublicMorozov_WaitEvent(uint32_t *notified_flags, TickType_t ticks_to_wait) {
            return WaitEvent(notified_flags, ticks_to_wait);
        }
        uint8_t *PublicMorozov_Get_station_rssi() {
            return &station_rssi;
        }
        uint8_t *PublicMorozov_Get_station_connect_failures() {
            return &station_connect_failures;
        }
        void PublicMorozov_StationTask(RequestItem *request) {
            StationTask(request);
        }
        void PublicMorozov_ScannerTask(RequestItem *request) {
            ScannerTask(request);
        }
        void PublicMorozov_AccessPointTask(RequestItem *request) {
            AccessPointTask(request);
        }
        void PublicMorozov_AddScannedSsid(const char *ssid, uint8_t rssi) {
            AddScannedSsid(ssid, rssi);
        }
        bool PublicMorozov_FindScannedSsid(const char *ssid, uint8_t *rssi) {
            return FindScannedSsid(ssid, rssi);
        }
        uint8_t PublicMorozov_ScaleRssiToPercent04(
            int8_t rssi,
            CurrentSettings::wifi_scanner_settings *scanner_settings) {
            return ScaleRssiToPercent04(rssi,
                                        scanner_settings->max_rssi,
                                        scanner_settings->min_rssi);
        }
        void PublicMorozov_AddApClient(const char *ssid, t_mac mac) {
            AddApClient(ssid, mac);
        }
        size_t PublicMorozov_GetApClientsCount(const char *ssid) {
            return GetApClientsCount(ssid);
        }
        void PublicMorozov_RemoveApClient(const char *ssid, t_mac mac) {
            RemoveApClient(ssid, mac);
        }
        void PublicMorozov_RemoveApClients(const char *ssid) {
            RemoveApClients(ssid);
        }
        static void PublicMorozov_ap_connect_wifi_event_handler(void *arg,
                                                                esp_event_base_t event_base,
                                                                int32_t event_id,
                                                                void *event_data) {
            ap_connect_wifi_event_handler(arg, event_base, event_id, event_data);
        }
        static void PublicMorozov_ap_disconnect_wifi_event_handler(void *arg,
                                                                   esp_event_base_t event_base,
                                                                   int32_t event_id,
                                                                   void *event_data) {
            ap_disconnect_wifi_event_handler(arg, event_base, event_id, event_data);
        }

        size_t PublicMorozov_Get_ap_clients_count() {
            return ap_clients_count;
        }
    };
} // namespace

TEST(WiFiServiceTestsGroup, ConnectToStation_requests_are_unique) {
    TestableWiFiService testable;
    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 0)
        .ignoreOtherParameters();
    char wakeup_bit[32];
    sprintf(wakeup_bit, "0x%08X", WiFiService::WAKEUP_BIT);
    mock(wakeup_bit).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->Size());

    testable.ConnectToStation();
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->Size());

    testable.ConnectToStation();
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->Size());
}

TEST(WiFiServiceTestsGroup, Scan_requests_are_unique) {
    TestableWiFiService testable;
    mock()
        .expectNCalls(3, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 0)
        .ignoreOtherParameters();
    char wakeup_bit[32];
    sprintf(wakeup_bit, "0x%08X", WiFiService::WAKEUP_BIT);
    mock(wakeup_bit).expectNCalls(3, "xEventGroupSetBits").ignoreOtherParameters();

    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->Size());

    testable.Scan("ssid_0");
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->Size());

    testable.Scan("ssid_1");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->Size());

    testable.Scan("ssid_1");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->Size());

    testable.Scan("ssid_0");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->Size());

    testable.Scan("ssid_2");
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests()->Size());
}

TEST(WiFiServiceTestsGroup, Scan_return_status) {
    TestableWiFiService testable;
    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 0)
        .ignoreOtherParameters();
    char wakeup_bit[32];
    sprintf(wakeup_bit, "0x%08X", WiFiService::WAKEUP_BIT);
    mock(wakeup_bit).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    const char *ssid_0 = "test_0";

    CHECK_EQUAL(LogicElement::MinValue, testable.Scan(ssid_0));
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->Size());

    testable.PublicMorozov_AddScannedSsid(ssid_0, 42);

    CHECK_EQUAL(42, testable.Scan(ssid_0));
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->Size());
}

TEST(WiFiServiceTestsGroup, CancelScan_raises_SCAN_BREAK_BIT) {
    TestableWiFiService testable;
    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 0)
        .ignoreOtherParameters();
    char wakeup_bit[32];
    sprintf(wakeup_bit, "0x%08X", WiFiService::WAKEUP_BIT);
    mock(wakeup_bit).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();
    char break_bit[32];
    sprintf(break_bit, "0x%08X", WiFiService::SCAN_BREAK_BIT);
    mock(break_bit).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    testable.Scan("ssid_0");

    testable.CancelScan("ssid_0");
}

TEST(WiFiServiceTestsGroup, DisconnectFromStation_removes_queued_station_request) {
    TestableWiFiService testable;
    mock()
        .expectNCalls(2, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 0)
        .ignoreOtherParameters();
    char wakeup_bit[32];
    sprintf(wakeup_bit, "0x%08X", WiFiService::WAKEUP_BIT);
    mock(wakeup_bit).expectNCalls(2, "xEventGroupSetBits").ignoreOtherParameters();
    char break_bit[32];
    sprintf(break_bit, "0x%08X", WiFiService::STA_BREAK_BIT);
    mock(break_bit).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    testable.ConnectToStation();
    testable.Scan("ssid_0");

    testable.DisconnectFromStation();

    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->Size());
    CHECK_EQUAL(RequestItemType::wqi_Scanner, testable.PublicMorozov_Get_requests()->First()->Type);
}

TEST(WiFiServiceTestsGroup, CancelScan_removes_queued_scan_request) {
    TestableWiFiService testable;
    mock()
        .expectNCalls(2, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 0)
        .ignoreOtherParameters();
    char wakeup_bit[32];
    sprintf(wakeup_bit, "0x%08X", WiFiService::WAKEUP_BIT);
    mock(wakeup_bit).expectNCalls(2, "xEventGroupSetBits").ignoreOtherParameters();
    char break_bit[32];
    sprintf(break_bit, "0x%08X", WiFiService::SCAN_BREAK_BIT);
    mock(break_bit).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    const char *ssid_0 = "ssid_0";
    const char *ssid_1 = "ssid_1";
    testable.Scan(ssid_0);
    testable.Scan(ssid_1);

    testable.CancelScan(ssid_0);

    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->Size());
    CHECK_EQUAL(ssid_1, testable.PublicMorozov_Get_requests()->First()->Payload.Scanner.ssid);
}

TEST(WiFiServiceTestsGroup, CancelAccessPoint_removes_queued_access_point_request) {
    TestableWiFiService testable;
    mock()
        .expectNCalls(2, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 0)
        .ignoreOtherParameters();
    char wakeup_bit[32];
    sprintf(wakeup_bit, "0x%08X", WiFiService::WAKEUP_BIT);
    mock(wakeup_bit).expectNCalls(2, "xEventGroupSetBits").ignoreOtherParameters();
    char break_bit[32];
    sprintf(break_bit, "0x%08X", WiFiService::AP_BREAK_BIT);
    mock(break_bit).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    const char *ssid_0 = "ssid_0";
    const char *ssid_1 = "ssid_1";
    const char *password = "password";
    const char *mac = "*";
    testable.AccessPoint(ssid_0, password, mac);
    testable.AccessPoint(ssid_1, password, mac);

    testable.CancelAccessPoint(ssid_0, password, mac);

    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->Size());
    CHECK_EQUAL(ssid_1, testable.PublicMorozov_Get_requests()->First()->Payload.AccessPoint.ssid);
}

TEST(WiFiServiceTestsGroup, RaiseSettingsChanged_raises_SETTINGS_CHANGED_BIT) {
    TestableWiFiService testable;
    char settings_changed_bit[32];
    sprintf(settings_changed_bit, "0x%08X", WiFiService::SETTINGS_CHANGED_BIT);
    mock(settings_changed_bit).expectOneCall("xEventGroupSetBits").ignoreOtherParameters();

    testable.RaiseSettingsChanged();
}

TEST(WiFiServiceTestsGroup, ConnectToStation_clears_STA_BREAK_BIT_and_keeps_other_break_bits) {
    TestableWiFiService testable;
    mock()
        .expectOneCall("xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 0)
        .ignoreOtherParameters()
        .andReturnValue((uint32_t)(WiFiService::STA_BREAK_BIT | WiFiService::SCAN_BREAK_BIT
                                   | WiFiService::AP_BREAK_BIT));
    char kept_bits[32];
    sprintf(kept_bits, "0x%08X", WiFiService::SCAN_BREAK_BIT | WiFiService::AP_BREAK_BIT);
    mock(kept_bits).expectOneCall("xEventGroupSetBits").ignoreOtherParameters();

    testable.ConnectToStation();
}

TEST(WiFiServiceTestsGroup, Scan_clears_SCAN_BREAK_BIT_and_keeps_other_break_bits) {
    TestableWiFiService testable;
    mock()
        .expectOneCall("xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 0)
        .ignoreOtherParameters()
        .andReturnValue((uint32_t)(WiFiService::STA_BREAK_BIT | WiFiService::SCAN_BREAK_BIT
                                   | WiFiService::AP_BREAK_BIT));
    char kept_bits[32];
    sprintf(kept_bits, "0x%08X", WiFiService::STA_BREAK_BIT | WiFiService::AP_BREAK_BIT);
    mock(kept_bits).expectOneCall("xEventGroupSetBits").ignoreOtherParameters();

    testable.Scan("ssid_0");
}

TEST(WiFiServiceTestsGroup, AccessPoint_clears_AP_BREAK_BIT_and_keeps_other_break_bits) {
    TestableWiFiService testable;
    mock()
        .expectOneCall("xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 0)
        .ignoreOtherParameters()
        .andReturnValue((uint32_t)(WiFiService::STA_BREAK_BIT | WiFiService::SCAN_BREAK_BIT
                                   | WiFiService::AP_BREAK_BIT));
    char kept_bits[32];
    sprintf(kept_bits, "0x%08X", WiFiService::STA_BREAK_BIT | WiFiService::SCAN_BREAK_BIT);
    mock(kept_bits).expectOneCall("xEventGroupSetBits").ignoreOtherParameters();

    testable.AccessPoint("ssid_0", NULL, NULL);
}

TEST(WiFiServiceTestsGroup, Scan_wakes_up_task_when_no_other_break_bits_to_keep) {
    TestableWiFiService testable;
    mock()
        .expectOneCall("xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 0)
        .ignoreOtherParameters()
        .andReturnValue((uint32_t)(WiFiService::SCAN_BREAK_BIT | WiFiService::STA_CONNECTED_BIT));
    char wakeup_bit[32];
    sprintf(wakeup_bit, "0x%08X", WiFiService::WAKEUP_BIT);
    mock(wakeup_bit).expectOneCall("xEventGroupSetBits").ignoreOtherParameters();

    testable.Scan("ssid_0");
}

TEST(WiFiServiceTestsGroup, WaitEvent_reads_and_clears_all_bits) {
    TestableWiFiService testable;
    mock()
        .expectOneCall("xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor", 0x00FFFFFF)
        .withIntParameter("xClearOnExit", pdTRUE)
        .withIntParameter("xWaitForAllBits", pdFALSE)
        .withUnsignedIntParameter("xTicksToWait", 42)
        .ignoreOtherParameters()
        .andReturnValue((uint32_t)WiFiService::STA_CONNECTED_BIT);

    uint32_t notified_flags = 0;
    CHECK_TRUE(testable.PublicMorozov_WaitEvent(&notified_flags, 42));
    CHECK_EQUAL(WiFiService::STA_CONNECTED_BIT, notified_flags);
}

TEST(WiFiServiceTestsGroup, WaitEvent_returns_false_on_timeout) {
    TestableWiFiService testable;
    mock().expectOneCall("xEventGroupWaitBits").ignoreOtherParameters().andReturnValue(0U);

    uint32_t notified_flags = 42;
    CHECK_FALSE(testable.PublicMorozov_WaitEvent(&notified_flags, 42));
    CHECK_EQUAL(0, notified_flags);
}

TEST(WiFiServiceTestsGroup, AccessPoint_requests_are_unique) {
    TestableWiFiService testable;
    mock()
        .expectNCalls(3, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 0)
        .ignoreOtherParameters();
    char wakeup_bit[32];
    sprintf(wakeup_bit, "0x%08X", WiFiService::WAKEUP_BIT);
    mock(wakeup_bit).expectNCalls(3, "xEventGroupSetBits").ignoreOtherParameters();

    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->Size());

    testable.AccessPoint("ssid_0", NULL, NULL);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->Size());

    testable.AccessPoint("ssid_1", NULL, NULL);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->Size());

    testable.AccessPoint("ssid_1", NULL, NULL);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->Size());

    testable.AccessPoint("ssid_0", NULL, NULL);
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->Size());

    testable.AccessPoint("ssid_2", NULL, NULL);
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests()->Size());
}

TEST(WiFiServiceTestsGroup, StationTask_waits_for_new_request_if_no_stored_wifi_creds) {
    TestableWiFiService testable;
    *testable.PublicMorozov_Get_station_rssi() = 200;

    settings.wifi_station.ssid[0] = 0;
    settings.wifi_station.password[0] = 0;

    mock().expectNoCall("esp_wifi_start");
    mock()
        .expectOneCall("xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", portMAX_DELAY)
        .ignoreOtherParameters()
        .andReturnValue((uint32_t)WiFiService::SETTINGS_CHANGED_BIT);
    RequestItem request = { RequestItemType::wqi_Station, {} };
    testable.PublicMorozov_StationTask(&request);
    CHECK_EQUAL(LogicElement::MinValue, *testable.PublicMorozov_Get_station_rssi());
}

TEST(WiFiServiceTestsGroup, StationTask_calls_connect) {
    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_STA)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");

    uint32_t notifVal = WiFiService::STA_BREAK_BIT;
    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 5000 / portTICK_PERIOD_MS)
        .ignoreOtherParameters()
        .andReturnValue(notifVal);

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 500 / portTICK_PERIOD_MS)
        .ignoreOtherParameters();

    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    TestableWiFiService testable;

    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");

    RequestItem request = { RequestItemType::wqi_Station, {} };
    testable.PublicMorozov_StationTask(&request);
}

TEST(WiFiServiceTestsGroup, StationTask_and_if_no_another_request_then_loop_forever) {
    mock().expectNCalls(13, "httpd_register_uri_handler").ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_STA)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();

    uint32_t notifVal = WiFiService::STA_CONNECTED_BIT;
    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 5000 / portTICK_PERIOD_MS)
        .ignoreOtherParameters()
        .andReturnValue(notifVal);

    mock()
        .expectNCalls(10, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 5000 / portTICK_PERIOD_MS)
        .ignoreOtherParameters()
        .andReturnValue((uint32_t)WiFiService::WAKEUP_BIT);

    uint32_t notifStop = WiFiService::STA_BREAK_BIT;
    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 5000 / portTICK_PERIOD_MS)
        .ignoreOtherParameters()
        .andReturnValue(notifStop);

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 500 / portTICK_PERIOD_MS)
        .ignoreOtherParameters();

    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    mock().expectNCalls(1, "esp_wifi_sta_get_ap_info").ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    TestableWiFiService testable;

    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");

    RequestItem request = { RequestItemType::wqi_Station, {} };
    testable.PublicMorozov_StationTask(&request);
}

TEST(WiFiServiceTestsGroup,
     StationTask_and_if_has_another_request_and_connection_occured_then_break_loop) {
    mock().expectNCalls(13, "httpd_register_uri_handler").ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_STA)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");
    mock().expectNCalls(2, "esp_timer_get_time").ignoreOtherParameters();

    uint32_t notifVal = WiFiService::STA_CONNECTED_BIT;
    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 5000 / portTICK_PERIOD_MS)
        .ignoreOtherParameters()
        .andReturnValue(notifVal);

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 500 / portTICK_PERIOD_MS)
        .ignoreOtherParameters();

    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    mock().expectNCalls(1, "esp_wifi_sta_get_ap_info").ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    TestableWiFiService testable;

    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");

    const char *ssid_0 = "test_0";
    testable.PublicMorozov_Get_requests()->Scan(ssid_0);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->Size());

    RequestItem request = { RequestItemType::wqi_Station, {} };
    testable.PublicMorozov_StationTask(&request);
    RequestItem request_1 = { RequestItemType::wqi_Scanner, { ssid_0 } };
    CHECK_TRUE(testable.PublicMorozov_Get_requests()->Contains(&request_1));
}

static void ExpectFailedStationRun(int runs) {
    mock().expectNCalls(runs, "esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock()
        .expectNCalls(runs, "esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_STA)
        .ignoreOtherParameters();
    mock().expectNCalls(runs, "esp_wifi_start");

    static uint32_t failed_flags = WiFiService::STA_FAILED_BIT;
    mock()
        .expectNCalls(runs, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 5000 / portTICK_PERIOD_MS)
        .ignoreOtherParameters()
        .andReturnValue(failed_flags);

    mock().expectNCalls(runs, "esp_wifi_disconnect");
    mock().expectNCalls(runs, "esp_wifi_stop");
    mock().expectNCalls(runs * 2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(runs * 2, "esp_event_handler_unregister").ignoreOtherParameters();

    mock()
        .expectNCalls(runs, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 500 / portTICK_PERIOD_MS)
        .ignoreOtherParameters();
}

TEST(WiFiServiceTestsGroup, StationTask_on_FAILED_stops_station_without_reconnect) {
    ExpectFailedStationRun(1);

    static uint64_t os_us = 0;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us, sizeof(os_us));
    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 3000 / portTICK_PERIOD_MS)
        .ignoreOtherParameters();

    TestableWiFiService testable;

    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");

    RequestItem request = { RequestItemType::wqi_Station, {} };
    testable.PublicMorozov_StationTask(&request);
    CHECK_EQUAL(1, *testable.PublicMorozov_Get_station_connect_failures());
    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->Size());
}

TEST(WiFiServiceTestsGroup, StationTask_on_FAILED_skips_reconnect_delay_if_has_another_request) {
    ExpectFailedStationRun(1);

    static uint64_t os_us = 0;
    mock()
        .expectNCalls(1, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us, sizeof(os_us));

    TestableWiFiService testable;

    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");

    testable.PublicMorozov_Get_requests()->Scan("test_0");

    RequestItem request = { RequestItemType::wqi_Station, {} };
    testable.PublicMorozov_StationTask(&request);
}

TEST(WiFiServiceTestsGroup, StationTask_wakes_ladder_once_when_station_becomes_unavailable) {
    const int retries_num_before_no_station = 3;
    const int runs = retries_num_before_no_station + 2;
    ExpectFailedStationRun(runs);

    static uint64_t os_us = 0;
    mock()
        .expectNCalls(runs * 2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us, sizeof(os_us));
    mock()
        .expectNCalls(runs, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", 3000 / portTICK_PERIOD_MS)
        .ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    TestableWiFiService testable;
    *testable.PublicMorozov_Get_station_rssi() = 200;

    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");

    RequestItem request = { RequestItemType::wqi_Station, {} };

    for (int i = 0; i < retries_num_before_no_station; i++) {
        testable.PublicMorozov_StationTask(&request);
        CHECK_EQUAL(200, *testable.PublicMorozov_Get_station_rssi());
        CHECK_EQUAL(i + 1, *testable.PublicMorozov_Get_station_connect_failures());
    }

    testable.PublicMorozov_StationTask(&request);
    CHECK_EQUAL(LogicElement::MinValue, *testable.PublicMorozov_Get_station_rssi());

    testable.PublicMorozov_StationTask(&request);
    CHECK_EQUAL(LogicElement::MinValue, *testable.PublicMorozov_Get_station_rssi());
}

TEST(WiFiServiceTestsGroup, StationTask_reconnect_delay_excludes_disconnection_wait) {
    ExpectFailedStationRun(1);

    static uint64_t disconnection_start_time_us = 0;
    mock()
        .expectOneCall("esp_timer_get_time")
        .withOutputParameterReturning("os_us",
                                      &disconnection_start_time_us,
                                      sizeof(disconnection_start_time_us));
    static uint64_t disconnection_end_time_us = 500 * 1000;
    mock()
        .expectOneCall("esp_timer_get_time")
        .withOutputParameterReturning("os_us",
                                      &disconnection_end_time_us,
                                      sizeof(disconnection_end_time_us));
    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("xTicksToWait", (3000 - 500) / portTICK_PERIOD_MS)
        .ignoreOtherParameters();

    TestableWiFiService testable;

    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");

    RequestItem request = { RequestItemType::wqi_Station, {} };
    testable.PublicMorozov_StationTask(&request);
}

TEST(WiFiServiceTestsGroup, ScannerTask_handle_SCAN_BREAK_BIT_and_then_stop_task) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock().expectOneCall("esp_wifi_start");
    mock()
        .expectOneCall("esp_wifi_scan_start")
        .withBoolParameter("block", false)
        .ignoreOtherParameters();

    uint64_t os_us = 0;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us, sizeof(os_us));
    mock().expectNCalls(1, "esp_wifi_scan_get_ap_records").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_wifi_scan_stop").ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");

    uint32_t notifVal = WiFiService::SCAN_BREAK_BIT;
    mock().expectNCalls(1, "xEventGroupWaitBits").ignoreOtherParameters().andReturnValue(notifVal);

    const char *ssid_0 = "test_0";
    RequestItem request = { RequestItemType::wqi_Scanner, { ssid_0 } };
    testable.PublicMorozov_ScannerTask(&request);
}

TEST(WiFiServiceTestsGroup, ScannerTask_before_stop_calls_WakeupProcessTask) {
    TestableWiFiService testable;
    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock().expectOneCall("esp_wifi_start");
    mock()
        .expectOneCall("esp_wifi_scan_start")
        .withBoolParameter("block", false)
        .ignoreOtherParameters();
    uint64_t os_us_start = 0;
    mock()
        .expectOneCall("esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us_start, sizeof(os_us_start));
    uint64_t os_us_after_wait = 7000000;
    mock()
        .expectOneCall("esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us_after_wait, sizeof(os_us_after_wait));
    uint16_t number = 0;
    mock()
        .expectOneCall("esp_wifi_scan_get_ap_records")
        .withOutputParameterReturning("number", &number, sizeof(number))
        .ignoreOtherParameters();
    mock().expectNCalls(1, "esp_wifi_scan_stop").ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    mock().expectNCalls(1, "xEventGroupWaitBits").ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    RequestItem request = { RequestItemType::wqi_Scanner, { ssid_0 } };
    testable.PublicMorozov_ScannerTask(&request);
}

TEST(WiFiServiceTestsGroup, ScannerTask_break_scan_by_timeout) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock().expectOneCall("esp_wifi_start");
    mock()
        .expectOneCall("esp_wifi_scan_start")
        .withBoolParameter("block", false)
        .ignoreOtherParameters();
    uint64_t os_us_start = 0;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us_start, sizeof(os_us_start));
    uint64_t os_us_after_wait = 7000000;
    mock()
        .expectOneCall("esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us_after_wait, sizeof(os_us_after_wait));

    uint16_t number = 0;
    mock()
        .expectNCalls(2, "esp_wifi_scan_get_ap_records")
        .withOutputParameterReturning("number", &number, sizeof(number))
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_scan_stop").ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");

    uint32_t notifVal = 0;
    mock().expectNCalls(2, "xEventGroupWaitBits").ignoreOtherParameters().andReturnValue(notifVal);

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    testable.PublicMorozov_Get_requests()->Scan(ssid_0);

    uint8_t rssi;
    RequestItem request = { RequestItemType::wqi_Scanner, { ssid_0 } };
    testable.PublicMorozov_ScannerTask(&request);
    CHECK_FALSE(testable.PublicMorozov_FindScannedSsid(ssid_0, &rssi));
}

TEST(WiFiServiceTestsGroup, ScannerTask_add_ssid_to_scanned_list_when_rssi_is_usable) {
    TestableWiFiService testable;

    mock().expectNCalls(2, "esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock().expectNCalls(2, "esp_wifi_start");
    mock()
        .expectNCalls(2, "esp_wifi_scan_start")
        .withBoolParameter("block", false)
        .ignoreOtherParameters();
    mock().expectNCalls(3, "esp_timer_get_time").ignoreOtherParameters();

    uint16_t number = 1;
    wifi_ap_record_t ap_records = {};

    mock()
        .expectNCalls(2, "esp_wifi_scan_get_ap_records")
        .withOutputParameterReturning("number", &number, sizeof(number))
        .withOutputParameterReturning("ap_records", &ap_records, sizeof(ap_records))
        .ignoreOtherParameters();
    mock().expectNCalls(2, "esp_wifi_scan_stop").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_wifi_disconnect");
    mock().expectNCalls(2, "esp_wifi_stop");

    uint32_t notifStop = WiFiService::SCAN_BREAK_BIT;
    mock().expectNCalls(1, "xEventGroupWaitBits").ignoreOtherParameters().andReturnValue(notifStop);

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(2, "xEventGroupSetBits").ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    testable.PublicMorozov_Get_requests()->Scan(ssid_0);

    strcpy((char *)ap_records.ssid, ssid_0);
    ap_records.rssi = -120;

    uint8_t rssi;
    RequestItem request = { RequestItemType::wqi_Scanner, { ssid_0 } };
    testable.PublicMorozov_ScannerTask(&request);
    CHECK_FALSE(testable.PublicMorozov_FindScannedSsid(ssid_0, &rssi));

    ap_records.rssi = -119;
    testable.PublicMorozov_ScannerTask(&request);
    CHECK_TRUE(testable.PublicMorozov_FindScannedSsid(ssid_0, &rssi));
    CHECK_EQUAL(2, rssi);
}

TEST(WiFiServiceTestsGroup, AccessPointTask_handle_AP_BREAK_BIT_and_then_stop_task) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_AP);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_AP)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    uint32_t notifVal = WiFiService::AP_BREAK_BIT;
    mock().expectNCalls(1, "xEventGroupWaitBits").ignoreOtherParameters().andReturnValue(notifVal);

    const char *ssid_0 = "test_0";
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    testable.PublicMorozov_AccessPointTask(&request);
}

TEST(WiFiServiceTestsGroup, AccessPointTask_does_not_recreates_request_if_canceled) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_AP);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_AP)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    uint32_t notifStop = WiFiService::AP_BREAK_BIT;
    mock().expectNCalls(1, "xEventGroupWaitBits").ignoreOtherParameters().andReturnValue(notifStop);

    const char *ssid_0 = "test_0";
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    testable.PublicMorozov_AccessPointTask(&request);
    CHECK_EQUAL_TEXT(0,
                     testable.PublicMorozov_Get_requests()->Size(),
                     "AccessPoint cannot be restarted");
}

TEST(WiFiServiceTestsGroup, AccessPointTask_cancellation_takes_precedence) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_AP);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_AP)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    uint32_t notifStop = WiFiService::AP_BREAK_BIT;
    mock().expectNCalls(1, "xEventGroupWaitBits").ignoreOtherParameters().andReturnValue(notifStop);

    const char *ssid_0 = "test_0";
    testable.PublicMorozov_Get_requests()->Station();
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    testable.PublicMorozov_AccessPointTask(&request);
    CHECK_FALSE(testable.PublicMorozov_Get_requests()->Contains(&request));
}

TEST(WiFiServiceTestsGroup, AccessPointTask_and_if_no_another_request_then_loop_forever) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_AP);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_AP)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    mock().expectNCalls(10, "xEventGroupWaitBits").ignoreOtherParameters();

    uint32_t notifStop = WiFiService::AP_BREAK_BIT;
    mock().expectNCalls(1, "xEventGroupWaitBits").ignoreOtherParameters().andReturnValue(notifStop);

    const char *ssid_0 = "test_0";
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    testable.PublicMorozov_AccessPointTask(&request);
}

TEST(WiFiServiceTestsGroup, AccessPointTask_break_by_timeout_only_if_other_requests) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_AP);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_AP)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    mock().expectNCalls(1, "xEventGroupWaitBits").ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    testable.PublicMorozov_Get_requests()->Station();
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    testable.PublicMorozov_AccessPointTask(&request);
}

TEST(WiFiServiceTestsGroup, AccessPointTask_denies_access_if_configured_as_insecure) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_AP);

    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_AP)
        .withUnsignedIntParameter("conf_ap_max_connection", 0)
        .ignoreOtherParameters();

    mock().expectOneCall("esp_wifi_start");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    uint32_t notifStop = WiFiService::AP_BREAK_BIT;
    mock().expectNCalls(1, "xEventGroupWaitBits").ignoreOtherParameters().andReturnValue(notifStop);

    const char *ssid_0 = "test_0";
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    testable.PublicMorozov_AccessPointTask(&request);
}

TEST(WiFiServiceTestsGroup, AccessPointTask_allow_access_if_configured_as_secure) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_AP);

    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_AP)
        .withUnsignedIntParameter("conf_ap_max_connection", 4)
        .ignoreOtherParameters();

    mock().expectOneCall("esp_wifi_start");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    uint32_t notifStop = WiFiService::AP_BREAK_BIT;
    mock().expectNCalls(1, "xEventGroupWaitBits").ignoreOtherParameters().andReturnValue(notifStop);

    const char *ssid_0 = "test_0";
    const char *passw_0 = "1234";
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    request.Payload.AccessPoint.password = passw_0;
    testable.PublicMorozov_AccessPointTask(&request);
}

TEST(WiFiServiceTestsGroup, ScaleRssiToPercent04) {
    TestableWiFiService testable;
    CurrentSettings::wifi_scanner_settings scanner_settings = { 0, -26, -120 };

    CHECK_EQUAL(0, testable.PublicMorozov_ScaleRssiToPercent04(-128, &scanner_settings));
    CHECK_EQUAL(0, testable.PublicMorozov_ScaleRssiToPercent04(-120, &scanner_settings));
    CHECK_EQUAL(2, testable.PublicMorozov_ScaleRssiToPercent04(-119, &scanner_settings));
    CHECK_EQUAL(127, testable.PublicMorozov_ScaleRssiToPercent04(-73, &scanner_settings));
    CHECK_EQUAL(252, testable.PublicMorozov_ScaleRssiToPercent04(-27, &scanner_settings));
    CHECK_EQUAL(255, testable.PublicMorozov_ScaleRssiToPercent04(-26, &scanner_settings));
    CHECK_EQUAL(255, testable.PublicMorozov_ScaleRssiToPercent04(0, &scanner_settings));
    CHECK_EQUAL(255, testable.PublicMorozov_ScaleRssiToPercent04(127, &scanner_settings));
}

TEST(WiFiServiceTestsGroup, AddScannedSsid_update_rssi_if_record_already_exists) {
    TestableWiFiService testable;

    uint8_t rssi;
    const char *ssid_0 = "test_0";

    testable.PublicMorozov_AddScannedSsid(ssid_0, 42);
    CHECK_TRUE(testable.PublicMorozov_FindScannedSsid(ssid_0, &rssi));
    CHECK_EQUAL(42, rssi);

    testable.PublicMorozov_AddScannedSsid(ssid_0, 19);
    CHECK_TRUE(testable.PublicMorozov_FindScannedSsid(ssid_0, &rssi));
    CHECK_EQUAL(19, rssi);
}

TEST(WiFiServiceTestsGroup, AddApClient_increase_clients_count_on_every_addition) {
    TestableWiFiService testable;

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";
    const t_mac mac_0 = { 0x0123456789AB };
    const t_mac mac_1 = { 0x0123456789AC };
    const t_mac mac_2 = { 0x0123456789AD };

    testable.PublicMorozov_AddApClient(ssid_0, mac_0);
    CHECK_EQUAL(1, testable.PublicMorozov_GetApClientsCount(ssid_0));

    testable.PublicMorozov_AddApClient(ssid_0, mac_1);
    CHECK_EQUAL(2, testable.PublicMorozov_GetApClientsCount(ssid_0));

    testable.PublicMorozov_AddApClient(ssid_0, mac_2);
    CHECK_EQUAL(3, testable.PublicMorozov_GetApClientsCount(ssid_0));

    testable.PublicMorozov_AddApClient(ssid_1, mac_0);
    CHECK_EQUAL(1, testable.PublicMorozov_GetApClientsCount(ssid_1));

    testable.PublicMorozov_AddApClient(ssid_1, mac_1);
    CHECK_EQUAL(2, testable.PublicMorozov_GetApClientsCount(ssid_1));

    testable.PublicMorozov_AddApClient(ssid_1, mac_2);
    CHECK_EQUAL(3, testable.PublicMorozov_GetApClientsCount(ssid_1));
}

TEST(WiFiServiceTestsGroup, set_of_ap_clients_is_unique) {
    TestableWiFiService testable;

    const char *ssid_0 = "test_0";
    const t_mac mac_0 = { 0x0123456789AB };
    const t_mac mac_1 = { 0x0123456789AC };

    testable.PublicMorozov_AddApClient(ssid_0, mac_0);
    CHECK_EQUAL(1, testable.PublicMorozov_GetApClientsCount(ssid_0));
    testable.PublicMorozov_AddApClient(ssid_0, mac_0);
    CHECK_EQUAL(1, testable.PublicMorozov_GetApClientsCount(ssid_0));

    testable.PublicMorozov_AddApClient(ssid_0, mac_1);
    CHECK_EQUAL(2, testable.PublicMorozov_GetApClientsCount(ssid_0));
    testable.PublicMorozov_AddApClient(ssid_0, mac_1);
    CHECK_EQUAL(2, testable.PublicMorozov_GetApClientsCount(ssid_0));

    testable.PublicMorozov_RemoveApClient(ssid_0, mac_0);
    CHECK_EQUAL(1, testable.PublicMorozov_GetApClientsCount(ssid_0));
    testable.PublicMorozov_AddApClient(ssid_0, mac_0);
    CHECK_EQUAL(2, testable.PublicMorozov_GetApClientsCount(ssid_0));
}

TEST(WiFiServiceTestsGroup,
     RemoveApClient_decrease_clients_count_and_erase_ssid_when_down_to_zero) {
    TestableWiFiService testable;

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";
    const t_mac mac_0 = { 0x0123456789AB };
    const t_mac mac_1 = { 0x0123456789AC };
    const t_mac mac_2 = { 0x0123456789AD };

    testable.PublicMorozov_AddApClient(ssid_0, mac_0);
    testable.PublicMorozov_AddApClient(ssid_0, mac_1);
    testable.PublicMorozov_AddApClient(ssid_0, mac_2);
    CHECK_EQUAL(3, testable.PublicMorozov_GetApClientsCount(ssid_0));

    testable.PublicMorozov_AddApClient(ssid_1, mac_0);
    testable.PublicMorozov_AddApClient(ssid_1, mac_1);
    testable.PublicMorozov_AddApClient(ssid_1, mac_2);
    CHECK_EQUAL(3, testable.PublicMorozov_GetApClientsCount(ssid_1));

    testable.PublicMorozov_RemoveApClient(ssid_0, mac_0);
    CHECK_EQUAL(2, testable.PublicMorozov_GetApClientsCount(ssid_0));
    testable.PublicMorozov_RemoveApClient(ssid_1, mac_0);
    CHECK_EQUAL(2, testable.PublicMorozov_GetApClientsCount(ssid_1));

    testable.PublicMorozov_RemoveApClient(ssid_0, mac_1);
    CHECK_EQUAL(1, testable.PublicMorozov_GetApClientsCount(ssid_0));
    testable.PublicMorozov_RemoveApClient(ssid_1, mac_1);
    CHECK_EQUAL(1, testable.PublicMorozov_GetApClientsCount(ssid_1));

    CHECK_EQUAL(2, testable.PublicMorozov_Get_ap_clients_count());

    testable.PublicMorozov_RemoveApClient(ssid_0, mac_2);
    CHECK_EQUAL(0, testable.PublicMorozov_GetApClientsCount(ssid_0));

    CHECK_EQUAL(1, testable.PublicMorozov_Get_ap_clients_count());

    testable.PublicMorozov_RemoveApClient(ssid_1, mac_2);
    CHECK_EQUAL(0, testable.PublicMorozov_GetApClientsCount(ssid_1));

    CHECK_EQUAL(0, testable.PublicMorozov_Get_ap_clients_count());
}

TEST(WiFiServiceTestsGroup, RemoveApClients) {
    TestableWiFiService testable;

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";
    const t_mac mac_0 = { 0x0123456789AB };
    const t_mac mac_1 = { 0x0123456789AC };
    const t_mac mac_2 = { 0x0123456789AD };

    testable.PublicMorozov_AddApClient(ssid_0, mac_0);
    testable.PublicMorozov_AddApClient(ssid_0, mac_1);
    testable.PublicMorozov_AddApClient(ssid_0, mac_2);
    CHECK_EQUAL(3, testable.PublicMorozov_GetApClientsCount(ssid_0));

    testable.PublicMorozov_AddApClient(ssid_1, mac_0);
    testable.PublicMorozov_AddApClient(ssid_1, mac_1);
    testable.PublicMorozov_AddApClient(ssid_1, mac_2);
    CHECK_EQUAL(3, testable.PublicMorozov_GetApClientsCount(ssid_1));

    testable.PublicMorozov_RemoveApClients(ssid_0);
    CHECK_EQUAL(0, testable.PublicMorozov_GetApClientsCount(ssid_0));

    testable.PublicMorozov_RemoveApClients(ssid_1);
    CHECK_EQUAL(0, testable.PublicMorozov_GetApClientsCount(ssid_1));
}

TEST(WiFiServiceTestsGroup,
     AccessPoint_when_connect_then_calls_WakeupProcessTask_and_add_connected_client) {
    TestableWiFiService testable;

    const char *ssid = "test";
    const char *mac = "01**********";

    WiFiService::AccessPointEventArg ap_event_arg = { &testable, ssid, mac };
    wifi_event_ap_staconnected_t event_data = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0 };

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    TestableWiFiService::PublicMorozov_ap_connect_wifi_event_handler(&ap_event_arg,
                                                                     NULL,
                                                                     0,
                                                                     &event_data);

    CHECK_EQUAL(1, testable.PublicMorozov_GetApClientsCount(ssid));
}

TEST(WiFiServiceTestsGroup,
     AccessPoint_when_disconnect_then_calls_WakeupProcessTask_and_remove_connected_client) {
    TestableWiFiService testable;

    const char *ssid = "test";
    const char *mac = "01**********";

    WiFiService::AccessPointEventArg ap_event_arg = { &testable, ssid, mac };
    wifi_event_ap_staconnected_t event_data = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0 };

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    t_mac t_mac = {};
    memcpy(&t_mac, &event_data.mac, sizeof(event_data.mac));
    testable.PublicMorozov_AddApClient(ssid, t_mac);

    TestableWiFiService::PublicMorozov_ap_disconnect_wifi_event_handler(&ap_event_arg,
                                                                        NULL,
                                                                        0,
                                                                        &event_data);
    CHECK_EQUAL(0, testable.PublicMorozov_GetApClientsCount(ssid));
}

TEST(WiFiServiceTestsGroup, AccessPoint_reject_connection_for_unknown_mac) {
    TestableWiFiService testable;

    const char *ssid = "test";
    const char *mac = "012345678900";

    WiFiService::AccessPointEventArg ap_event_arg = { &testable, ssid, mac };
    wifi_event_ap_staconnected_t event_data = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 42 };

    mock()
        .expectNCalls(1, "esp_wifi_deauth_sta")
        .withUnsignedIntParameter("aid", 42)
        .ignoreOtherParameters();

    TestableWiFiService::PublicMorozov_ap_connect_wifi_event_handler(&ap_event_arg,
                                                                     NULL,
                                                                     0,
                                                                     &event_data);
}