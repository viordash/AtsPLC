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
    TEST_SETUP(){ mock().expectOneCall("xEventGroupWaitBits").ignoreOtherParameters();
mock().expectOneCall("vEventGroupDelete").ignoreOtherParameters();
Controller::Stop();
}

TEST_TEARDOWN() {
}
}
;

TEST(LogicControllerTestsGroup, SampleIOValues_AI) {
    volatile uint16_t adc = 100 / 0.1;
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock()
        .expectNCalls(1, "adc_read")
        .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));

    CHECK_EQUAL(0, Controller::GetAIRelativeValue());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(100 / 0.4, Controller::GetAIRelativeValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_DI) {
    mock("0").expectNCalls(1, "gpio_get_level").andReturnValue(0);
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();

    CHECK_EQUAL(LogicElement::MinValue, Controller::GetDIRelativeValue());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::GetDIRelativeValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_O1) {
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").andReturnValue(0);
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();

    CHECK_EQUAL(LogicElement::MinValue, Controller::GetO1RelativeValue());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::GetO1RelativeValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_O2) {
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").andReturnValue(0);
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();

    CHECK_EQUAL(LogicElement::MinValue, Controller::GetO2RelativeValue());
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::GetO2RelativeValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_V1) {
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();

    CHECK_EQUAL(0, Controller::GetV1RelativeValue());
    Controller::SetV1RelativeValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42, Controller::GetV1RelativeValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_V2) {
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();

    CHECK_EQUAL(0, Controller::GetV2RelativeValue());
    Controller::SetV2RelativeValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42, Controller::GetV2RelativeValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_V3) {
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();

    CHECK_EQUAL(0, Controller::GetV3RelativeValue());
    Controller::SetV3RelativeValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42, Controller::GetV3RelativeValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_V4) {
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();

    CHECK_EQUAL(0, Controller::GetV4RelativeValue());
    Controller::SetV4RelativeValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_EQUAL(42, Controller::GetV4RelativeValue());
}

TEST(LogicControllerTestsGroup, SampleIOValues_returns_status_of_value_change) {
    mock("0").expectNCalls(6, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(6, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(6, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(6, "adc_read").ignoreOtherParameters();

    Controller::SampleIOValues();
    CHECK_FALSE(Controller::SampleIOValues());
    Controller::SetV4RelativeValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_FALSE(Controller::SampleIOValues());
    Controller::SetV1RelativeValue(42);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_FALSE(Controller::SampleIOValues());
}

TEST(LogicControllerTestsGroup, GetIOValues) {
    mock("0").expectNCalls(1, "gpio_get_level").andReturnValue(0);
    mock("2").expectNCalls(1, "gpio_get_level").andReturnValue(0);
    mock("15").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    volatile uint16_t adc = 19 / 0.1;
    mock()
        .expectNCalls(1, "adc_read")
        .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));

    Controller::SetV1RelativeValue(1);
    Controller::SetV2RelativeValue(2);
    Controller::SetV3RelativeValue(3);
    Controller::SetV4RelativeValue(4);

    ControllerIOValues io_values = Controller::GetIOValues();

    CHECK_COMPARE(io_values.AI, !=, 19);
    CHECK_COMPARE(io_values.DI, !=, LogicElement::MaxValue);
    CHECK_COMPARE(io_values.O1, !=, LogicElement::MaxValue);
    CHECK_COMPARE(io_values.O2, !=, LogicElement::MaxValue);
    CHECK_COMPARE(io_values.V1, !=, 1);
    CHECK_COMPARE(io_values.V2, !=, 2);
    CHECK_COMPARE(io_values.V3, !=, 3);
    CHECK_COMPARE(io_values.V4, !=, 4);
    CHECK_TRUE(Controller::SampleIOValues());

    io_values = Controller::GetIOValues();
    CHECK_COMPARE(io_values.AI, ==, (uint8_t)(19 / 0.4));
    CHECK_COMPARE(io_values.DI, ==, LogicElement::MaxValue);
    CHECK_COMPARE(io_values.O1, ==, LogicElement::MaxValue);
    CHECK_COMPARE(io_values.O2, ==, LogicElement::MaxValue);
    CHECK_COMPARE(io_values.V1, ==, 1);
    CHECK_COMPARE(io_values.V2, ==, 2);
    CHECK_COMPARE(io_values.V3, ==, 3);
    CHECK_COMPARE(io_values.V4, ==, 4);
}
