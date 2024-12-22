#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/WiFi/WiFiService.h"
#include "main/settings.h"

TEST_GROUP(LogicWiFiServiceTestsGroup){ //
                                        TEST_SETUP(){}

                                        TEST_TEARDOWN(){}
};

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
        EventBits_t PublicMorozov_StationTask() {
            return StationTask();
        }
    };
} // namespace

TEST(LogicWiFiServiceTestsGroup, Scan_requests_are_unique) {
    mock().expectOneCall("xEventGroupWaitBits").ignoreOtherParameters();

    TestableWiFiService testable;
    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(3, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

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

TEST(LogicWiFiServiceTestsGroup, Generate_requests_are_unique) {
    mock().expectOneCall("xEventGroupWaitBits").ignoreOtherParameters();

    TestableWiFiService testable;
    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(3, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

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
    mock().expectOneCall("xEventGroupWaitBits").ignoreOtherParameters();

    TestableWiFiService testable;

    settings.wifi.ssid[0] = 0;
    settings.wifi.password[0] = 0;

    EventBits_t uxBits = testable.PublicMorozov_StationTask();
    CHECK_EQUAL(0, uxBits);
}

TEST(LogicWiFiServiceTestsGroup, StationTask_calls_connect) {
    mock().expectOneCall("xTaskGetTickCount").ignoreOtherParameters();
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
                                      | WiFiService::STOP_BIT | WiFiService::NEW_REQUEST_BIT)
        .andReturnValue(WiFiService::STOP_BIT)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor", WiFiService::STARTED_BIT)
        .ignoreOtherParameters();

    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");

    TestableWiFiService testable;

    strcpy(settings.wifi.ssid, "test_ssid");
    strcpy(settings.wifi.password, "test_pwd");

    EventBits_t uxBits = testable.PublicMorozov_StationTask();
    CHECK_EQUAL(WiFiService::STOP_BIT, uxBits);
}

TEST(LogicWiFiServiceTestsGroup, StationTask_new_request_break_loop) {
    mock().expectNCalls(2, "xTaskGetTickCount").ignoreOtherParameters();
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
                                      | WiFiService::STOP_BIT | WiFiService::NEW_REQUEST_BIT)
        .andReturnValue(WiFiService::CONNECTED_BIT)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::CONNECTED_BIT | WiFiService::FAILED_BIT
                                      | WiFiService::STOP_BIT | WiFiService::NEW_REQUEST_BIT)
        .andReturnValue(WiFiService::NEW_REQUEST_BIT)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor", WiFiService::STARTED_BIT)
        .ignoreOtherParameters();

    mock().expectOneCall("esp_wifi_disconnect");
    mock().expectOneCall("esp_wifi_stop");

    TestableWiFiService testable;

    strcpy(settings.wifi.ssid, "test_ssid");
    strcpy(settings.wifi.password, "test_pwd");

    EventBits_t uxBits = testable.PublicMorozov_StationTask();
    CHECK_EQUAL(WiFiService::NEW_REQUEST_BIT, uxBits);
}

TEST(LogicWiFiServiceTestsGroup, StationTask_if_FAILED_then_reconnect) {
    mock().expectNCalls(2, "xTaskGetTickCount").ignoreOtherParameters();
    mock().expectNCalls(1, "vTaskDelayUntil").ignoreOtherParameters();
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
                                      | WiFiService::STOP_BIT | WiFiService::NEW_REQUEST_BIT)
        .andReturnValue(WiFiService::FAILED_BIT)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor",
                                  WiFiService::CONNECTED_BIT | WiFiService::FAILED_BIT
                                      | WiFiService::STOP_BIT | WiFiService::NEW_REQUEST_BIT)
        .andReturnValue(WiFiService::STOP_BIT)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xEventGroupWaitBits")
        .withUnsignedIntParameter("uxBitsToWaitFor", WiFiService::STARTED_BIT)
        .ignoreOtherParameters();

    mock().expectNCalls(2, "esp_wifi_disconnect");
    mock().expectNCalls(2, "esp_wifi_stop");

    TestableWiFiService testable;

    strcpy(settings.wifi.ssid, "test_ssid");
    strcpy(settings.wifi.password, "test_pwd");

    EventBits_t uxBits = testable.PublicMorozov_StationTask();
    CHECK_EQUAL(WiFiService::STOP_BIT, uxBits);
}