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

TEST(LogicControllerTestsGroup, SampleIOValues_AI) {
    volatile uint16_t adc = 100 / 0.1;
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_timer_get_time").ignoreOtherParameters();
    mock()
        .expectNCalls(2, "adc_read")
        .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));

    Controller::SampleIOValues();
    CHECK_FALSE(Controller::SampleIOValues());

    adc = 42 / 0.1;
    Controller::AI.GetValue();
    Controller::RemoveRequestWakeupMs((void *)&Controller::AI);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42 / 0.4, Controller::AI.PeekValue());
    CHECK_FALSE(Controller::SampleIOValues());
}

TEST(LogicControllerTestsGroup, SampleIOValues_DI) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").andReturnValue(1);
    mock("0").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    Controller::SampleIOValues();
    CHECK_FALSE(Controller::SampleIOValues());

    Controller::DI.GetValue();
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::DI.PeekValue());
    CHECK_FALSE(Controller::SampleIOValues());
}

TEST(LogicControllerTestsGroup, SampleIOValues_O1) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").andReturnValue(1);
    mock("2").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    Controller::SampleIOValues();
    CHECK_FALSE(Controller::SampleIOValues());

    Controller::O1.GetValue();
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::O1.PeekValue());
    CHECK_FALSE(Controller::SampleIOValues());
}

TEST(LogicControllerTestsGroup, SampleIOValues_O2) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").andReturnValue(1);
    mock("15").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    Controller::SampleIOValues();
    CHECK_FALSE(Controller::SampleIOValues());

    Controller::O2.GetValue();
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::O2.PeekValue());
    CHECK_FALSE(Controller::SampleIOValues());
}

TEST(LogicControllerTestsGroup, SampleIOValues_V1) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();

    Controller::SampleIOValues();
    CHECK_FALSE(Controller::SampleIOValues());

    Controller::V1.GetValue();
    Controller::V1.SetValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42, Controller::V1.PeekValue());
    CHECK_FALSE(Controller::SampleIOValues());
}

TEST(LogicControllerTestsGroup, SampleIOValues_V2) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();

    Controller::SampleIOValues();
    CHECK_FALSE(Controller::SampleIOValues());

    Controller::V2.GetValue();
    Controller::V2.SetValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42, Controller::V2.PeekValue());
    CHECK_FALSE(Controller::SampleIOValues());
}

TEST(LogicControllerTestsGroup, SampleIOValues_V3) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();

    Controller::SampleIOValues();
    CHECK_FALSE(Controller::SampleIOValues());

    Controller::V3.GetValue();
    Controller::V3.SetValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42, Controller::V3.PeekValue());
    CHECK_FALSE(Controller::SampleIOValues());
}

TEST(LogicControllerTestsGroup, SampleIOValues_V4) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();

    Controller::SampleIOValues();
    CHECK_FALSE(Controller::SampleIOValues());

    Controller::V4.GetValue();
    Controller::V4.SetValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42, Controller::V4.PeekValue());
    CHECK_FALSE(Controller::SampleIOValues());
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
                wifi_service->PublicMorozov_Get_requests()->front().type);
}
