#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Controller.h"
#include "main/WiFi/WiFiService.h"
#include "main/settings.h"

TEST_GROUP(LogicWiFiServiceTestsGroup){
    //
    TEST_SETUP(){ mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
mock().expectOneCall("xTaskCreate").ignoreOtherParameters();
Controller::Start(NULL, NULL);
}

TEST_TEARDOWN() {
    Controller::Stop();
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
        void PublicMorozov_StationTask(RequestItem *request) {
            StationTask(request);
        }
        void PublicMorozov_ScannerTask(RequestItem *request) {
            ScannerTask(request);
        }
        void PublicMorozov_AccessPointTask(RequestItem *request) {
            AccessPointTask(request);
        }
        void PublicMorozov_AddSsidToScannedList(const char *ssid, uint8_t rssi) {
            AddSsidToScannedList(ssid, rssi);
        }
        bool PublicMorozov_FindSsidInScannedList(const char *ssid, uint8_t *rssi) {
            return FindSsidInScannedList(ssid, rssi);
        }
        uint8_t PublicMorozov_ScaleRssiToPercent04(int8_t rssi,
                                                   CurrentSettings::wifi_scanner_settings *scanner_settings) {
            return ScaleRssiToPercent04(rssi, scanner_settings);
        }
    };
} // namespace

TEST(LogicWiFiServiceTestsGroup, ConnectToStation_requests_are_unique) {
    TestableWiFiService testable;
    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->size());

    testable.ConnectToStation();
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());

    testable.ConnectToStation();
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());
}

TEST(LogicWiFiServiceTestsGroup, Scan_requests_are_unique) {
    TestableWiFiService testable;
    mock()
        .expectNCalls(3, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->size());

    testable.Scan("ssid_0");
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());

    testable.Scan("ssid_1");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->size());

    testable.Scan("ssid_1");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->size());

    testable.Scan("ssid_0");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->size());

    testable.Scan("ssid_2");
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests()->size());
}

TEST(LogicWiFiServiceTestsGroup, Scan_return_status) {
    TestableWiFiService testable;
    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    const char *ssid_0 = "test_0";

    CHECK_EQUAL(LogicElement::MinValue, testable.Scan(ssid_0));
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());

    testable.PublicMorozov_AddSsidToScannedList(ssid_0, 42);

    CHECK_EQUAL(42, testable.Scan(ssid_0));
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());
}

TEST(LogicWiFiServiceTestsGroup, CancelScan) {
    TestableWiFiService testable;
    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", WiFiService::CANCEL_REQUEST_BIT)
        .withIntParameter("eAction", eNotifyAction::eSetBits)
        .ignoreOtherParameters();

    CHECK_EQUAL(LogicElement::MinValue, testable.Scan("ssid_0"));
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());

    testable.CancelScan("ssid_0");
    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->size());
}

TEST(LogicWiFiServiceTestsGroup, Generate_requests_are_unique) {
    TestableWiFiService testable;
    mock()
        .expectNCalls(3, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->size());

    testable.Generate("ssid_0");
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());

    testable.Generate("ssid_1");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->size());

    testable.Generate("ssid_1");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->size());

    testable.Generate("ssid_0");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->size());

    testable.Generate("ssid_2");
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests()->size());
}

TEST(LogicWiFiServiceTestsGroup, StationTask_returns_immediatelly_if_no_stored_wifi_creds) {
    TestableWiFiService testable;

    settings.wifi_station.ssid[0] = 0;
    settings.wifi_station.password[0] = 0;

    mock().expectNoCall("esp_wifi_start");
    mock().expectNoCall("xEventGroupWaitBits");
    RequestItem request = { RequestItemType::wqi_Station, {} };
    testable.PublicMorozov_StationTask(&request);
}

TEST(LogicWiFiServiceTestsGroup, StationTask_calls_connect) {
    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_STA)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");

    uint32_t notifVal = WiFiService::STOP_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit",
                                  WiFiService::CANCEL_REQUEST_BIT | WiFiService::CONNECTED_BIT
                                      | WiFiService::FAILED_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifVal, sizeof(notifVal))
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit",
                                  WiFiService::CONNECTED_BIT | WiFiService::FAILED_BIT)
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

TEST(LogicWiFiServiceTestsGroup,
     StationTask_and_if_one_more_request_and_connection_occured_then_break_loop) {
    mock().expectNCalls(3, "httpd_register_uri_handler").ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_STA)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");

    uint32_t notifVal = WiFiService::CONNECTED_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit",
                                  WiFiService::CANCEL_REQUEST_BIT | WiFiService::CONNECTED_BIT
                                      | WiFiService::FAILED_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifVal, sizeof(notifVal))
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit",
                                  WiFiService::CONNECTED_BIT | WiFiService::FAILED_BIT)
        .ignoreOtherParameters();

    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    TestableWiFiService testable;

    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");

    testable.PublicMorozov_Get_requests()->Station();
    testable.PublicMorozov_Get_requests()->Scan("ssid");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->size());

    RequestItem request = { RequestItemType::wqi_Station, {} };
    testable.PublicMorozov_StationTask(&request);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());
}

TEST(LogicWiFiServiceTestsGroup, StationTask_if_FAILED_then_reconnect) {
    mock().expectNCalls(2, "esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock()
        .expectNCalls(2, "esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_STA)
        .ignoreOtherParameters();
    mock().expectNCalls(2, "esp_wifi_start");

    uint32_t notifVal = WiFiService::FAILED_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit",
                                  WiFiService::CANCEL_REQUEST_BIT | WiFiService::CONNECTED_BIT
                                      | WiFiService::FAILED_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifVal, sizeof(notifVal))
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit",
                                  WiFiService::CANCEL_REQUEST_BIT | WiFiService::CONNECTED_BIT
                                      | WiFiService::FAILED_BIT)
        .ignoreOtherParameters()
        .andReturnValue(pdFALSE);

    uint32_t notifStop = WiFiService::STOP_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit",
                                  WiFiService::CANCEL_REQUEST_BIT | WiFiService::CONNECTED_BIT
                                      | WiFiService::FAILED_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifStop, sizeof(notifStop))
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit",
                                  WiFiService::CONNECTED_BIT | WiFiService::FAILED_BIT)
        .ignoreOtherParameters();

    mock().expectNCalls(2, "esp_wifi_disconnect");
    mock().expectNCalls(2, "esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    TestableWiFiService testable;

    strcpy(settings.wifi_station.ssid, "test_ssid");
    strcpy(settings.wifi_station.password, "test_pwd");

    RequestItem request = { RequestItemType::wqi_Station, {} };
    testable.PublicMorozov_StationTask(&request);
}

TEST(
    LogicWiFiServiceTestsGroup,
    ScannerTask_handle_CANCEL_REQUEST_BIT_and_then_stop_task_only_if_request_has_already_been_deleted) {
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

    uint32_t notifVal = WiFiService::CANCEL_REQUEST_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit", WiFiService::CANCEL_REQUEST_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifVal, sizeof(notifVal))
        .ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";
    testable.PublicMorozov_Get_requests()->Scan(ssid_1);

    RequestItem request = { RequestItemType::wqi_Scanner, { ssid_0 } };
    testable.PublicMorozov_ScannerTask(&request);
}

TEST(LogicWiFiServiceTestsGroup, ScannerTask_ignore_CANCEL_REQUEST_BIT_for_other_scan_requests) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock().expectOneCall("esp_wifi_start");
    mock()
        .expectOneCall("esp_wifi_scan_start")
        .withBoolParameter("block", false)
        .ignoreOtherParameters();
    uint64_t os_us = 0;
    mock()
        .expectNCalls(3, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us, sizeof(os_us));
    uint16_t number = 0;
    mock()
        .expectNCalls(2, "esp_wifi_scan_get_ap_records")
        .withOutputParameterReturning("number", &number, sizeof(number))
        .ignoreOtherParameters();
    mock().expectNCalls(1, "esp_wifi_scan_stop").ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");

    uint32_t notifVal = WiFiService::CANCEL_REQUEST_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit", WiFiService::CANCEL_REQUEST_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifVal, sizeof(notifVal))
        .ignoreOtherParameters();

    uint32_t notifStop = WiFiService::STOP_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit", WiFiService::CANCEL_REQUEST_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifStop, sizeof(notifStop))
        .ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    testable.PublicMorozov_Get_requests()->Scan(ssid_0);

    RequestItem request = { RequestItemType::wqi_Scanner, { ssid_0 } };
    testable.PublicMorozov_ScannerTask(&request);
}

TEST(LogicWiFiServiceTestsGroup, ScannerTask_before_stop_calls_WakeupProcessTask) {
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

    uint32_t notifStop = WiFiService::STOP_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit", WiFiService::CANCEL_REQUEST_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifStop, sizeof(notifStop))
        .ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    RequestItem request = { RequestItemType::wqi_Scanner, { ssid_0 } };
    testable.PublicMorozov_ScannerTask(&request);
}

TEST(LogicWiFiServiceTestsGroup, ScannerTask_break_scan_by_timeout) {
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
    mock()
        .expectNCalls(2, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit", WiFiService::CANCEL_REQUEST_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifVal, sizeof(notifVal))
        .ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    testable.PublicMorozov_Get_requests()->Scan(ssid_0);

    uint8_t rssi;
    RequestItem request = { RequestItemType::wqi_Scanner, { ssid_0 } };
    testable.PublicMorozov_ScannerTask(&request);
    CHECK_FALSE(testable.PublicMorozov_FindSsidInScannedList(ssid_0, &rssi));
}

TEST(LogicWiFiServiceTestsGroup, ScannerTask_add_ssid_to_scanned_list_when_rssi_is_usable) {
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

    uint32_t notifStop = WiFiService::STOP_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit", WiFiService::CANCEL_REQUEST_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifStop, sizeof(notifStop))
        .ignoreOtherParameters();

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
    CHECK_FALSE(testable.PublicMorozov_FindSsidInScannedList(ssid_0, &rssi));

    ap_records.rssi = -119;
    testable.PublicMorozov_ScannerTask(&request);
    CHECK_TRUE(testable.PublicMorozov_FindSsidInScannedList(ssid_0, &rssi));
    CHECK_EQUAL(2, rssi);
}

TEST(
    LogicWiFiServiceTestsGroup,
    AccessPointTask_handle_CANCEL_REQUEST_BIT_and_then_stop_task_only_if_request_has_already_been_deleted) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_AP);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_AP)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");
    mock().expectOneCall("esp_wifi_stop");

    uint32_t notifVal = WiFiService::CANCEL_REQUEST_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit", WiFiService::CANCEL_REQUEST_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifVal, sizeof(notifVal))
        .ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";
    testable.PublicMorozov_Get_requests()->AccessPoint(ssid_1);

    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    testable.PublicMorozov_AccessPointTask(&request);
}

TEST(LogicWiFiServiceTestsGroup, AccessPointTask_ignore_CANCEL_REQUEST_BIT_for_other_AP_requests) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_AP);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_AP)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");
    mock().expectOneCall("esp_wifi_stop");

    uint32_t notifVal = WiFiService::CANCEL_REQUEST_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit", WiFiService::CANCEL_REQUEST_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifVal, sizeof(notifVal))
        .ignoreOtherParameters();

    uint32_t notifStop = WiFiService::STOP_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit", WiFiService::CANCEL_REQUEST_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifStop, sizeof(notifStop))
        .ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    testable.PublicMorozov_Get_requests()->AccessPoint(ssid_0);

    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    testable.PublicMorozov_AccessPointTask(&request);
}

TEST(LogicWiFiServiceTestsGroup, AccessPointTask_before_stop_calls_WakeupProcessTask) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_AP);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_AP)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");
    mock().expectOneCall("esp_wifi_stop");

    uint32_t notifStop = WiFiService::STOP_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit", WiFiService::CANCEL_REQUEST_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifStop, sizeof(notifStop))
        .ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    testable.PublicMorozov_AccessPointTask(&request);
}

TEST(LogicWiFiServiceTestsGroup, AccessPointTask_recreates_request_for_further_restart) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_AP);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_AP)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");
    mock().expectOneCall("esp_wifi_stop");

    uint32_t notifStop = WiFiService::STOP_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit", WiFiService::CANCEL_REQUEST_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifStop, sizeof(notifStop))
        .ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    testable.PublicMorozov_AccessPointTask(&request);
    CHECK_EQUAL_TEXT(1,
                     testable.PublicMorozov_Get_requests()->size(),
                     "AccessPoint can be restarted");
}

TEST(LogicWiFiServiceTestsGroup, AccessPointTask_does_not_recreates_request_if_canceled) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_AP);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_AP)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");
    mock().expectOneCall("esp_wifi_stop");

    uint32_t notifStop = WiFiService::CANCEL_REQUEST_BIT;
    mock()
        .expectNCalls(1, "xTaskNotifyWait")
        .withUnsignedIntParameter("ulBitsToClearOnExit", WiFiService::CANCEL_REQUEST_BIT)
        .withOutputParameterReturning("pulNotificationValue", &notifStop, sizeof(notifStop))
        .ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    testable.PublicMorozov_AccessPointTask(&request);
    CHECK_EQUAL_TEXT(0,
                     testable.PublicMorozov_Get_requests()->size(),
                     "AccessPoint cannot be restarted");
}

TEST(LogicWiFiServiceTestsGroup, ScaleRssiToPercent04) {
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

TEST(LogicWiFiServiceTestsGroup, AddSsidToScannedList_update_rssi_if_record_already_exists) {
    TestableWiFiService testable;

    uint8_t rssi;
    const char *ssid_0 = "test_0";

    testable.PublicMorozov_AddSsidToScannedList(ssid_0, 42);
    CHECK_TRUE(testable.PublicMorozov_FindSsidInScannedList(ssid_0, &rssi));
    CHECK_EQUAL(42, rssi);

    testable.PublicMorozov_AddSsidToScannedList(ssid_0, 19);
    CHECK_TRUE(testable.PublicMorozov_FindSsidInScannedList(ssid_0, &rssi));
    CHECK_EQUAL(19, rssi);
}