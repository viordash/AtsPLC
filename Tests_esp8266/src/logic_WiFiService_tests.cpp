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
        EventGroupHandle_t PublicMorozov_Get_event() {
            return event;
        }
        void PublicMorozov_StationTask() {
            StationTask();
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
        uint8_t PublicMorozov_ScaleRssiToPercent04(int8_t rssi) {
            return ScaleRssiToPercent04(rssi);
        }
    };
} // namespace

TEST(LogicWiFiServiceTestsGroup, TryConnectToStation_requests_are_unique) {
    TestableWiFiService testable;
    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(1, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->size());

    testable.ConnectToStation();
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());

    testable.ConnectToStation();
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());
}

TEST(LogicWiFiServiceTestsGroup, Scan_requests_are_unique) {
    TestableWiFiService testable;
    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(3, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    mock().expectNCalls(3, "esp_timer_get_time").ignoreOtherParameters();

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
    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(1, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();

    const char *ssid_0 = "test_0";

    CHECK_EQUAL(LogicElement::MinValue, testable.Scan(ssid_0));
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());

    testable.PublicMorozov_AddSsidToScannedList(ssid_0, 42);

    CHECK_EQUAL(42, testable.Scan(ssid_0));
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());
}

TEST(LogicWiFiServiceTestsGroup, Scan_re_add_request_only_after_delay) {
    TestableWiFiService testable;
    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(2, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(3, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (uint64_t *)&os_us, sizeof(os_us));

    const char *ssid_0 = "test_0";

    testable.Scan(ssid_0);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());

    testable.PublicMorozov_Get_requests()->RemoveScanner(ssid_0);

    os_us = 2999000;
    testable.Scan(ssid_0);
    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->size());

    os_us = 3000000;
    Controller::RemoveExpiredWakeupRequests();
    testable.Scan(ssid_0);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());
}

TEST(LogicWiFiServiceTestsGroup, CancelScan) {
    TestableWiFiService testable;
    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(1, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    sprintf(buffer, "0x%08X", WiFiService::CANCEL_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(1, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();

    CHECK_EQUAL(LogicElement::MinValue, testable.Scan("ssid_0"));
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());

    testable.CancelScan("ssid_0");
    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->size());
}

TEST(LogicWiFiServiceTestsGroup, Generate_requests_are_unique) {
    TestableWiFiService testable;
    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(3, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    mock().expectNCalls(3, "esp_timer_get_time").ignoreOtherParameters();

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

TEST(LogicWiFiServiceTestsGroup, Generate_re_add_request_only_after_delay) {
    TestableWiFiService testable;
    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(2, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(3, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (uint64_t *)&os_us, sizeof(os_us));

    const char *ssid_0 = "test_0";

    testable.Generate(ssid_0);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());

    testable.PublicMorozov_Get_requests()->RemoveAccessPoint(ssid_0);

    os_us = 2999000;
    testable.Generate(ssid_0);
    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->size());

    os_us = 3000000;
    Controller::RemoveExpiredWakeupRequests();
    testable.Generate(ssid_0);
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());
}

TEST(LogicWiFiServiceTestsGroup, StationTask_returns_immediatelly_if_no_stored_wifi_creds) {
    TestableWiFiService testable;

    settings.wifi.ssid[0] = 0;
    settings.wifi.password[0] = 0;

    mock().expectNoCall("esp_wifi_start");
    mock().expectNoCall("xEventGroupWaitBits");
    testable.PublicMorozov_StationTask();
}

TEST(LogicWiFiServiceTestsGroup, StationTask_calls_connect) {
    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_STA)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::CONNECTED_BIT | WiFiService::FAILED_BIT
                                      | WiFiService::STOP_BIT | WiFiService::NEW_REQUEST_BIT
                                      | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::STOP_BIT)
        .ignoreOtherParameters();

    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    TestableWiFiService testable;

    strcpy(settings.wifi.ssid, "test_ssid");
    strcpy(settings.wifi.password, "test_pwd");

    testable.PublicMorozov_StationTask();
}

TEST(LogicWiFiServiceTestsGroup,
     StationTask_new_request_break_loop_and_call_StationDone_for_requests) {
    mock().expectNCalls(3, "httpd_register_uri_handler").ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_STA)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::CONNECTED_BIT | WiFiService::FAILED_BIT
                                      | WiFiService::STOP_BIT | WiFiService::NEW_REQUEST_BIT
                                      | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::CONNECTED_BIT)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::FAILED_BIT | WiFiService::STOP_BIT
                                      | WiFiService::NEW_REQUEST_BIT
                                      | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::NEW_REQUEST_BIT)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::FAILED_BIT | WiFiService::STOP_BIT
                                      | WiFiService::NEW_REQUEST_BIT
                                      | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::NEW_REQUEST_BIT)
        .ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    TestableWiFiService testable;

    strcpy(settings.wifi.ssid, "test_ssid");
    strcpy(settings.wifi.password, "test_pwd");

    testable.ConnectToStation();
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());

    testable.PublicMorozov_StationTask();
    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->size());
}

TEST(LogicWiFiServiceTestsGroup, StationTask_if_FAILED_then_reconnect) {
    mock().expectNCalls(2, "esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_STA);
    mock()
        .expectNCalls(2, "esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_STA)
        .ignoreOtherParameters();
    mock().expectNCalls(2, "esp_wifi_start");

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::CONNECTED_BIT | WiFiService::FAILED_BIT
                                      | WiFiService::STOP_BIT | WiFiService::NEW_REQUEST_BIT
                                      | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::FAILED_BIT)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::STOP_BIT | WiFiService::NEW_REQUEST_BIT
                                      | WiFiService::CANCEL_REQUEST_BIT)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::CONNECTED_BIT | WiFiService::FAILED_BIT
                                      | WiFiService::STOP_BIT | WiFiService::NEW_REQUEST_BIT
                                      | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::NEW_REQUEST_BIT)
        .ignoreOtherParameters();

    mock().expectNCalls(2, "esp_wifi_disconnect");
    mock().expectNCalls(2, "esp_wifi_stop");
    mock().expectNCalls(2, "esp_event_handler_register").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_event_handler_unregister").ignoreOtherParameters();

    TestableWiFiService testable;

    strcpy(settings.wifi.ssid, "test_ssid");
    strcpy(settings.wifi.password, "test_pwd");

    testable.PublicMorozov_StationTask();
}

TEST(
    LogicWiFiServiceTestsGroup,
    ScannerTask_handle_CANCEL_REQUEST_BIT_and_then_stop_task_only_request_has_already_been_deleted) {
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
    mock().expectNCalls(1, "esp_wifi_scan_get_ap_records").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_wifi_scan_stop").ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");

    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(1, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::STOP_BIT | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::CANCEL_REQUEST_BIT)
        .ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";
    testable.Scan(ssid_1);

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
        .expectNCalls(4, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us, sizeof(os_us));
    uint16_t number = 0;
    mock()
        .expectNCalls(2, "esp_wifi_scan_get_ap_records")
        .withOutputParameterReturning("number", &number, sizeof(number))
        .ignoreOtherParameters();
    mock().expectNCalls(1, "esp_wifi_scan_stop").ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");

    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(1, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::STOP_BIT | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::CANCEL_REQUEST_BIT)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::STOP_BIT | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::STOP_BIT)
        .ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    testable.Scan(ssid_0);

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

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::STOP_BIT | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::STOP_BIT)
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
        .expectOneCall("esp_wifi_scan_get_ap_records")
        .withOutputParameterReturning("number", &number, sizeof(number))
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_scan_stop").ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");

    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(1, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::STOP_BIT | WiFiService::CANCEL_REQUEST_BIT)
        .ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    testable.Scan(ssid_0);

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
    mock().expectNCalls(4, "esp_timer_get_time").ignoreOtherParameters();

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

    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(1, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(2, "xEventGroupSetBits").ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::STOP_BIT | WiFiService::CANCEL_REQUEST_BIT)
        .ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    testable.Scan(ssid_0);

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
    AccessPointTask_handle_CANCEL_REQUEST_BIT_and_then_stop_task_only_request_has_already_been_deleted) {
    TestableWiFiService testable;

    mock().expectOneCall("esp_wifi_set_mode").withIntParameter("mode", WIFI_MODE_AP);
    mock()
        .expectOneCall("esp_wifi_set_config")
        .withIntParameter("interface", ESP_IF_WIFI_AP)
        .ignoreOtherParameters();
    mock().expectOneCall("esp_wifi_start");
    mock().expectOneCall("esp_wifi_stop");
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(1, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::STOP_BIT | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::CANCEL_REQUEST_BIT)
        .ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";
    testable.Generate(ssid_1);

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
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();

    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(1, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::STOP_BIT | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::CANCEL_REQUEST_BIT)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::STOP_BIT | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::STOP_BIT)
        .ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    testable.Generate(ssid_0);

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

    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::STOP_BIT | WiFiService::CANCEL_REQUEST_BIT)
        .andReturnValue(WiFiService::STOP_BIT)
        .ignoreOtherParameters();

    const char *ssid_0 = "test_0";
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    testable.PublicMorozov_AccessPointTask(&request);
}

TEST(LogicWiFiServiceTestsGroup, ScaleRssiToPercent04) {
    TestableWiFiService testable;

    CHECK_EQUAL(0, testable.PublicMorozov_ScaleRssiToPercent04(-128));
    CHECK_EQUAL(0, testable.PublicMorozov_ScaleRssiToPercent04(-120));
    CHECK_EQUAL(2, testable.PublicMorozov_ScaleRssiToPercent04(-119));
    CHECK_EQUAL(127, testable.PublicMorozov_ScaleRssiToPercent04(-73));
    CHECK_EQUAL(252, testable.PublicMorozov_ScaleRssiToPercent04(-27));
    CHECK_EQUAL(255, testable.PublicMorozov_ScaleRssiToPercent04(-26));
    CHECK_EQUAL(255, testable.PublicMorozov_ScaleRssiToPercent04(0));
    CHECK_EQUAL(255, testable.PublicMorozov_ScaleRssiToPercent04(127));
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