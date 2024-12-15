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

TEST_GROUP(LogicControllerTestsGroup){
    //
    TEST_SETUP(){ mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
mock().expectOneCall("xTaskCreate").ignoreOtherParameters();
Controller::Start(NULL);
}

TEST_TEARDOWN() {
    Controller::Stop();
}
}
;

TEST(LogicControllerTestsGroup, SampleIOValues_AI) {
    volatile uint16_t adc = 100 / 0.1;
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock()
        .expectNCalls(1, "adc_read")
        .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));
    Controller::AI.GetValue();

    CHECK_EQUAL(0, Controller::AI.PeekValue());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(100 / 0.4, Controller::AI.PeekValue());
    CHECK_FALSE(Controller::SampleIOValues());
}

TEST(LogicControllerTestsGroup, SampleIOValues_DI) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").andReturnValue(0);
    Controller::DI.GetValue();

    CHECK_EQUAL(LogicElement::MinValue, Controller::DI.PeekValue());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::DI.PeekValue());
    CHECK_FALSE(Controller::SampleIOValues());
}

TEST(LogicControllerTestsGroup, SampleIOValues_O1) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").andReturnValue(0);
    Controller::O1.GetValue();

    CHECK_EQUAL(LogicElement::MinValue, Controller::O1.PeekValue());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::O1.PeekValue());
    CHECK_FALSE(Controller::SampleIOValues());
}

TEST(LogicControllerTestsGroup, SampleIOValues_O2) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").andReturnValue(0);
    Controller::O2.GetValue();

    CHECK_EQUAL(LogicElement::MinValue, Controller::O2.PeekValue());
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

    Controller::V4.GetValue();
    Controller::V4.SetValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42, Controller::V4.PeekValue());
    CHECK_FALSE(Controller::SampleIOValues());
}
