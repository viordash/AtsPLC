#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "main/sys_gpio.cpp"

TEST_GROUP(GpioTestsGroup){ //
                            TEST_SETUP(){}

                            TEST_TEARDOWN(){}
};

TEST(GpioTestsGroup, gpio_init__on_startup) {
    int event = 0;
    mock().expectNCalls(2, "gpio_config").ignoreOtherParameters();
    mock().expectOneCall("adc_init").ignoreOtherParameters();
    mock().expectOneCall("xEventGroupCreate").andReturnValue(&event);
    mock().expectOneCall("gpio_install_isr_service");
    mock("0").expectOneCall("gpio_isr_handler_add");
    mock("12").expectOneCall("gpio_isr_handler_add");
    mock("13").expectOneCall("gpio_isr_handler_add");
    mock("14").expectOneCall("gpio_isr_handler_add");
    mock("16").expectOneCall("gpio_set_level").withUnsignedIntParameter("level", GPIO_PASSIVE);
    mock("2").expectOneCall("gpio_set_level").withUnsignedIntParameter("level", GPIO_PASSIVE);
    mock("15").expectOneCall("gpio_set_level").withUnsignedIntParameter("level", GPIO_PASSIVE);

    gpio_init();
}

static void test_BUTTON_XXX_isr_handler(const char *testable_gpio_num,
                                        const char *extra_gpio_num_0,
                                        const char *extra_gpio_num_1,
                                        const char *extra_gpio_num_2,
                                        uint32_t bits,
                                        bool opened) {
    int event = 42;
    gpio_isr_t isr_handler = NULL;
    char buffer[32];
    sprintf(buffer, "0x%08X", bits);

    mock().expectNCalls(2, "gpio_config").ignoreOtherParameters();
    mock().expectOneCall("adc_init").ignoreOtherParameters();
    mock().expectOneCall("xEventGroupCreate").andReturnValue(&event);
    mock().expectOneCall("gpio_install_isr_service");
    mock("16").expectOneCall("gpio_set_level").ignoreOtherParameters();
    mock("2").expectOneCall("gpio_set_level").ignoreOtherParameters();
    mock("15").expectOneCall("gpio_set_level").ignoreOtherParameters();

    mock(extra_gpio_num_0).expectOneCall("gpio_isr_handler_add");
    mock(extra_gpio_num_1).expectOneCall("gpio_isr_handler_add");
    mock(extra_gpio_num_2).expectOneCall("gpio_isr_handler_add");

    mock(testable_gpio_num).expectOneCall("gpio_isr_handler_add");
    mock(testable_gpio_num).setData("isr_handler", &isr_handler);
    mock(testable_gpio_num)
        .expectOneCall("gpio_get_level")
        .andReturnValue(opened ? INPUT_NO_VALUE : INPUT_NC_VALUE);
    mock(buffer)
        .expectOneCall("xEventGroupSetBitsFromISR")
        .withPointerParameter("xEventGroup", &event);

    gpio_init();
    isr_handler(NULL);
}

TEST(GpioTestsGroup, BUTTON_UP_IO_isr_handler__when_input_is_open) {
    test_BUTTON_XXX_isr_handler("13", "12", "14", "0", BUTTON_UP_IO_OPEN, true);
}

TEST(GpioTestsGroup, BUTTON_UP_IO_isr_handler__when_input_is_close) {
    test_BUTTON_XXX_isr_handler("13", "12", "14", "0", BUTTON_UP_IO_CLOSE, false);
}

TEST(GpioTestsGroup, BUTTON_DOWN_IO_isr_handler__when_input_is_open) {
    test_BUTTON_XXX_isr_handler("12", "13", "14", "0", BUTTON_DOWN_IO_OPEN, true);
}

TEST(GpioTestsGroup, BUTTON_DOWN_IO_isr_handler__when_input_is_close) {
    test_BUTTON_XXX_isr_handler("12", "13", "14", "0", BUTTON_DOWN_IO_CLOSE, false);
}

TEST(GpioTestsGroup, BUTTON_RIGHT_IO_isr_handler__when_input_is_open) {
    test_BUTTON_XXX_isr_handler("0",
                                "13",
                                "12",
                                "14",
                                BUTTON_RIGHT_IO_OPEN | INPUT_1_IO_OPEN,
                                true);
}

TEST(GpioTestsGroup, BUTTON_RIGHT_IO_isr_handler__when_input_is_close) {
    test_BUTTON_XXX_isr_handler("0",
                                "13",
                                "12",
                                "14",
                                BUTTON_RIGHT_IO_CLOSE | INPUT_1_IO_CLOSE,
                                false);
}

TEST(GpioTestsGroup, BUTTON_SELECT_IO_isr_handler__when_input_is_open) {
    test_BUTTON_XXX_isr_handler("14", "13", "12", "0", BUTTON_SELECT_IO_OPEN, true);
}

TEST(GpioTestsGroup, BUTTON_SELECT_IO_isr_handler__when_input_is_close) {
    test_BUTTON_XXX_isr_handler("14", "13", "12", "0", BUTTON_SELECT_IO_CLOSE, false);
}

TEST(GpioTestsGroup, set_OUTPUT_0_to_active) {
    mock("2").expectOneCall("gpio_set_level").withUnsignedIntParameter("level", GPIO_ACTIVE);
    mock("15").expectNoCall("gpio_set_level");

    set_digital_value(OUTPUT_0, true);
}

TEST(GpioTestsGroup, set_OUTPUT_0_to_passive) {
    mock("2").expectOneCall("gpio_set_level").withUnsignedIntParameter("level", GPIO_PASSIVE);
    mock("15").expectNoCall("gpio_set_level");

    set_digital_value(OUTPUT_0, false);
}

TEST(GpioTestsGroup, set_OUTPUT_1_to_active) {
    mock("2").expectNoCall("gpio_set_level");
    mock("15").expectOneCall("gpio_set_level").withUnsignedIntParameter("level", GPIO_ACTIVE);
    mock("16").expectOneCall("gpio_set_level").ignoreOtherParameters();

    set_digital_value(OUTPUT_1, true);
}

TEST(GpioTestsGroup, set_OUTPUT_1_to_passive) {
    mock("2").expectNoCall("gpio_set_level");
    mock("15").expectOneCall("gpio_set_level").withUnsignedIntParameter("level", GPIO_PASSIVE);
    mock("16").expectOneCall("gpio_set_level").ignoreOtherParameters();

    set_digital_value(OUTPUT_1, false);
}