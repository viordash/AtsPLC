#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/Datetime/DatetimeService.h"
#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Outputs/DecOutput.h"
#include "main/WiFi/WiFiService.h"

namespace {
    class TestableWiFiService : public WiFiService {
      public:
        WiFiRequests *PublicMorozov_Get_requests() {
            return &requests;
        }
    };
} // namespace
TestableWiFiService *wifi_service;
static DatetimeService *datetime_service;

TEST_GROUP(LogicControllerTestsGroup){
    //
    TEST_SETUP(){ mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
mock().expectOneCall("xTaskCreate").ignoreOtherParameters();
wifi_service = new TestableWiFiService();
datetime_service = new DatetimeService();
Controller::Start(NULL, wifi_service, NULL, datetime_service);
}

TEST_TEARDOWN() {
    Controller::Stop();
    delete wifi_service;
    delete datetime_service;
}
}
;

TEST(LogicControllerTestsGroup, FetchIOValues_AI) {
    volatile uint16_t adc = 100 / 0.1;
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock()
        .expectNCalls(1, "adc_read")
        .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));

    adc = 42 / 0.1;
    Controller::RemoveRequestWakeupMs((void *)&Controller::AI);

    CHECK_EQUAL(0, Controller::AI.PeekValue());
    Controller::FetchIOValues();
    CHECK_EQUAL(42 / 0.4, Controller::AI.PeekValue());
}

TEST(LogicControllerTestsGroup, FetchIOValues_DI) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    CHECK_EQUAL(LogicElement::MinValue, Controller::DI.PeekValue());
    Controller::FetchIOValues();
    CHECK_EQUAL(LogicElement::MaxValue, Controller::DI.PeekValue());
}

TEST(LogicControllerTestsGroup, FetchIOValues_O1) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    CHECK_EQUAL(LogicElement::MinValue, Controller::O1.PeekValue());
    Controller::FetchIOValues();
    CHECK_EQUAL(LogicElement::MaxValue, Controller::O1.PeekValue());
}

TEST(LogicControllerTestsGroup, FetchIOValues_O2) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    CHECK_EQUAL(LogicElement::MinValue, Controller::O2.PeekValue());
    Controller::FetchIOValues();
    CHECK_EQUAL(LogicElement::MaxValue, Controller::O2.PeekValue());
}

TEST(LogicControllerTestsGroup, FetchIOValues_V1_mandatory_after_init) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    Controller::V1.WriteValue(42);

    CHECK_EQUAL(LogicElement::MinValue, Controller::V1.PeekValue());
    Controller::FetchIOValues();
    CHECK_EQUAL(42, Controller::V1.PeekValue());
}

TEST(LogicControllerTestsGroup, FetchIOValues_V2_mandatory_after_init) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    Controller::V2.WriteValue(42);

    CHECK_EQUAL(LogicElement::MinValue, Controller::V2.PeekValue());
    Controller::FetchIOValues();
    CHECK_EQUAL(42, Controller::V2.PeekValue());
}

TEST(LogicControllerTestsGroup, FetchIOValues_V3_mandatory_after_init) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    Controller::V3.WriteValue(42);

    CHECK_EQUAL(LogicElement::MinValue, Controller::V3.PeekValue());
    Controller::FetchIOValues();
    CHECK_EQUAL(42, Controller::V3.PeekValue());
}

TEST(LogicControllerTestsGroup, FetchIOValues_V4_mandatory_after_init) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    Controller::V4.WriteValue(42);

    CHECK_EQUAL(LogicElement::MinValue, Controller::V4.PeekValue());
    Controller::FetchIOValues();
    CHECK_EQUAL(42, Controller::V4.PeekValue());
}

TEST(LogicControllerTestsGroup, WakeupProcessTask) {
    char buffer[32];
    sprintf(buffer, "0x%08X", Controller::WAKEUP_PROCESS_TASK);
    mock(buffer).expectNCalls(1, "xEventGroupSetBits").ignoreOtherParameters();

    Controller::WakeupProcessTask();
}