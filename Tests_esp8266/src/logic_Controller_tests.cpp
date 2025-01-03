#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Outputs/DecOutput.h"
#include "main/WiFi/WiFiService.h"

namespace {
    class TestableWiFiService : public WiFiService {
      public:
        WiFiRequests *PublicMorozov_Get_requests() {
            return &requests;
        }
        EventGroupHandle_t PublicMorozov_Get_event() {
            return event;
        }
    };
} // namespace
TestableWiFiService *wifi_service;

TEST_GROUP(LogicControllerTestsGroup){
    //
    TEST_SETUP(){ mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
mock().expectOneCall("xTaskCreate").ignoreOtherParameters();
wifi_service = new TestableWiFiService();
Controller::Start(NULL, wifi_service);
}

TEST_TEARDOWN() {
    Controller::Stop();
    delete wifi_service;
}
}
;

TEST(LogicControllerTestsGroup, FetchIOValues_AI) {
    volatile uint16_t adc = 100 / 0.1;
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_timer_get_time").ignoreOtherParameters();
    mock()
        .expectNCalls(2, "adc_read")
        .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));

    Controller::FetchIOValues();
    CHECK_FALSE(Controller::FetchIOValues());

    adc = 42 / 0.1;
    Controller::AI.ReadValue();
    Controller::RemoveRequestWakeupMs((void *)&Controller::AI);
    CHECK_TRUE(Controller::FetchIOValues());
    CHECK_EQUAL(42 / 0.4, Controller::AI.PeekValue());
    CHECK_FALSE(Controller::FetchIOValues());
}

TEST(LogicControllerTestsGroup, FetchIOValues_DI) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").andReturnValue(1);
    mock("0").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    Controller::FetchIOValues();
    CHECK_FALSE(Controller::FetchIOValues());

    Controller::DI.ReadValue();
    CHECK_TRUE(Controller::FetchIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::DI.PeekValue());
    CHECK_FALSE(Controller::FetchIOValues());
}

TEST(LogicControllerTestsGroup, FetchIOValues_O1) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").andReturnValue(1);
    mock("2").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    Controller::FetchIOValues();
    CHECK_FALSE(Controller::FetchIOValues());

    Controller::O1.ReadValue();
    CHECK_TRUE(Controller::FetchIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::O1.PeekValue());
    CHECK_FALSE(Controller::FetchIOValues());
}

TEST(LogicControllerTestsGroup, FetchIOValues_O2) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").andReturnValue(1);
    mock("15").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    Controller::FetchIOValues();
    CHECK_FALSE(Controller::FetchIOValues());

    Controller::O2.ReadValue();
    CHECK_TRUE(Controller::FetchIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::O2.PeekValue());
    CHECK_FALSE(Controller::FetchIOValues());
}

TEST(LogicControllerTestsGroup, FetchIOValues_V1_mandatory_after_init) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    Controller::V1.WriteValue(42);
    Controller::V1.CommitChanges();

    CHECK_TRUE(Controller::FetchIOValues());
    Controller::V1.ReadValue();
    CHECK_FALSE(Controller::FetchIOValues());
}

TEST(LogicControllerTestsGroup, FetchIOValues_V2_mandatory_after_init) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    Controller::V2.WriteValue(42);
    Controller::V2.CommitChanges();

    CHECK_TRUE(Controller::FetchIOValues());
    Controller::V2.ReadValue();
    CHECK_FALSE(Controller::FetchIOValues());
}

TEST(LogicControllerTestsGroup, FetchIOValues_V3_mandatory_after_init) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    Controller::V3.WriteValue(42);
    Controller::V3.CommitChanges();

    CHECK_TRUE(Controller::FetchIOValues());
    Controller::V3.ReadValue();
    CHECK_FALSE(Controller::FetchIOValues());
}

TEST(LogicControllerTestsGroup, FetchIOValues_V4_mandatory_after_init) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    Controller::V4.WriteValue(42);
    Controller::V4.CommitChanges();

    CHECK_TRUE(Controller::FetchIOValues());
    Controller::V4.ReadValue();
    CHECK_FALSE(Controller::FetchIOValues());
}

TEST(LogicControllerTestsGroup,
     UnbindVariable_for_last_variable_also_send_ConnectToStation_request_to_wifi_service) {

    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(1, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", wifi_service->PublicMorozov_Get_event());

    Controller::BindVariableToWiFi(MapIO::V1, "test_ssid");
    Controller::BindVariableToWiFi(MapIO::V2, "test_ssid");
    Controller::BindVariableToWiFi(MapIO::V3, "test_ssid");
    Controller::BindVariableToWiFi(MapIO::V4, "test_ssid");

    CHECK_EQUAL(0, wifi_service->PublicMorozov_Get_requests()->size());

    Controller::UnbindVariable(MapIO::V1);
    Controller::UnbindVariable(MapIO::V2);
    Controller::UnbindVariable(MapIO::V3);
    Controller::UnbindVariable(MapIO::V4);

    CHECK_EQUAL(1, wifi_service->PublicMorozov_Get_requests()->size());
    CHECK_EQUAL(RequestItemType::wqi_Station,
                wifi_service->PublicMorozov_Get_requests()->front().Type);
}

TEST(LogicControllerTestsGroup, WakeupProcessTask) {
    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    Controller::WakeupProcessTask();
}