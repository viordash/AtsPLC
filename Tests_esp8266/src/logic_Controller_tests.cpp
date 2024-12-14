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
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
    mock()
        .expectNCalls(1, "adc_read")
        .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));

    CHECK_EQUAL(0, Controller::AI.PeekValue());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(100 / 0.4, Controller::AI.PeekValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_DI) {
    mock("0").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    CHECK_EQUAL(LogicElement::MinValue, Controller::DI.PeekValue());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::DI.PeekValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_O1) {
    mock("2").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    CHECK_EQUAL(LogicElement::MinValue, Controller::O1.PeekValue());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::O1.PeekValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_O2) {
    mock("15").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    CHECK_EQUAL(LogicElement::MinValue, Controller::O2.PeekValue());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::O2.PeekValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_V1) {
    CHECK_EQUAL(0, Controller::V1.PeekValue());
    Controller::V1.SetValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42, Controller::V1.PeekValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_V2) {
    CHECK_EQUAL(0, Controller::V2.PeekValue());
    Controller::V2.SetValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42, Controller::V2.PeekValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_V3) {
    CHECK_EQUAL(0, Controller::V3.PeekValue());
    Controller::V3.SetValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42, Controller::V3.PeekValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_V4) {
    CHECK_EQUAL(0, Controller::V4.PeekValue());
    Controller::V4.SetValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42, Controller::V4.PeekValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_returns_status_of_value_change) {
    Controller::SampleIOValues();
    CHECK_FALSE(Controller::SampleIOValues());
    Controller::V4.SetValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_FALSE(Controller::SampleIOValues());
    Controller::V1.SetValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_FALSE(Controller::SampleIOValues());
}

// TEST(LogicControllerTestsGroup, GetIOValues) {
//     mock("0").expectNCalls(1, "gpio_get_level").andReturnValue(0);
//     mock("2").expectNCalls(1, "gpio_get_level").andReturnValue(0);
//     mock("15").expectNCalls(1, "gpio_get_level").andReturnValue(0);

//     mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();
//     volatile uint16_t adc = 19 / 0.1;
//     mock()
//         .expectNCalls(1, "adc_read")
//         .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));

//     Controller::SetV1RelativeValue(1);
//     Controller::SetV2RelativeValue(2);
//     Controller::SetV3RelativeValue(3);
//     Controller::SetV4RelativeValue(4);

//     ControllerIOValues io_values = Controller::GetIOValues();

//     CHECK_COMPARE(io_values.AI.value, !=, 19);
//     CHECK_COMPARE(io_values.DI.value, !=, LogicElement::MaxValue);
//     CHECK_COMPARE(io_values.O1.value, !=, LogicElement::MaxValue);
//     CHECK_COMPARE(io_values.O2.value, !=, LogicElement::MaxValue);
//     CHECK_COMPARE(io_values.V1, !=, 1);
//     CHECK_COMPARE(io_values.V2, !=, 2);
//     CHECK_COMPARE(io_values.V3, !=, 3);
//     CHECK_COMPARE(io_values.V4, !=, 4);

//     Controller::AI.PeekValue();
//     Controller::RemoveRequestWakeupMs((void *)Controller::AI.PeekValue);
//     Controller::DI.PeekValue();
//     Controller::O1.PeekValue();
//     Controller::O2.PeekValue();
//     CHECK_TRUE(Controller::SampleIOValues());

//     io_values = Controller::GetIOValues();
//     CHECK_COMPARE(io_values.AI.value, ==, (uint8_t)(19 / 0.4));
//     CHECK_COMPARE(io_values.DI.value, ==, LogicElement::MaxValue);
//     CHECK_COMPARE(io_values.O1.value, ==, LogicElement::MaxValue);
//     CHECK_COMPARE(io_values.O2.value, ==, LogicElement::MaxValue);
//     CHECK_COMPARE(io_values.V1, ==, 1);
//     CHECK_COMPARE(io_values.V2, ==, 2);
//     CHECK_COMPARE(io_values.V3, ==, 3);
//     CHECK_COMPARE(io_values.V4, ==, 4);
// }
