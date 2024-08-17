#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport_c.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "main/sys_gpio.c"

TEST_GROUP_C_SETUP(GpioTestsGroup) {
}
TEST_GROUP_C_TEARDOWN(GpioTestsGroup) {
}

TEST_C(GpioTestsGroup, gpio_init__on_startup) {
    int event = 0;
    mock_c()->expectNCalls(2, "gpio_config")->ignoreOtherParameters();
    mock_c()->expectOneCall("adc_init")->ignoreOtherParameters();
    mock_c()->expectOneCall("xEventGroupCreate")->andReturnPointerValue(&event);
    mock_c()->expectOneCall("gpio_install_isr_service");
    mock_scope_c("0")->expectOneCall("gpio_isr_handler_add");
    mock_scope_c("12")->expectOneCall("gpio_isr_handler_add");
    mock_scope_c("13")->expectOneCall("gpio_isr_handler_add");
    mock_scope_c("14")->expectOneCall("gpio_isr_handler_add");
    mock_scope_c("16")
        ->expectOneCall("gpio_set_level")
        ->withUnsignedIntParameters("level", GPIO_PASSIVE);
    mock_scope_c("2")
        ->expectOneCall("gpio_set_level")
        ->withUnsignedIntParameters("level", GPIO_PASSIVE);
    mock_scope_c("15")
        ->expectOneCall("gpio_set_level")
        ->withUnsignedIntParameters("level", GPIO_PASSIVE);

    gpio_init(0);
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

    mock_c()->expectNCalls(2, "gpio_config")->ignoreOtherParameters();
    mock_c()->expectOneCall("adc_init")->ignoreOtherParameters();
    mock_c()->expectOneCall("xEventGroupCreate")->andReturnPointerValue(&event);
    mock_c()->expectOneCall("gpio_install_isr_service");
    mock_scope_c("16")->expectOneCall("gpio_set_level")->ignoreOtherParameters();
    mock_scope_c("2")->expectOneCall("gpio_set_level")->ignoreOtherParameters();
    mock_scope_c("15")->expectOneCall("gpio_set_level")->ignoreOtherParameters();

    mock_scope_c(extra_gpio_num_0)->expectOneCall("gpio_isr_handler_add");
    mock_scope_c(extra_gpio_num_1)->expectOneCall("gpio_isr_handler_add");
    mock_scope_c(extra_gpio_num_2)->expectOneCall("gpio_isr_handler_add");

    mock_scope_c(testable_gpio_num)->expectOneCall("gpio_isr_handler_add");
    mock_scope_c(testable_gpio_num)->setPointerData("isr_handler", &isr_handler);
    mock_scope_c(testable_gpio_num)
        ->expectOneCall("gpio_get_level")
        ->andReturnIntValue(opened ? INPUT_NO_VALUE : INPUT_NC_VALUE);
    mock_scope_c(buffer)
        ->expectOneCall("xEventGroupSetBitsFromISR")
        ->withPointerParameters("xEventGroup", &event);

    gpio_init();
    isr_handler(NULL);
}

TEST_C(GpioTestsGroup, BUTTON_UP_IO_isr_handler__when_input_is_open) {
    test_BUTTON_XXX_isr_handler("13", "12", "14", "0", BUTTON_UP_IO_OPEN, true);
}

TEST_C(GpioTestsGroup, BUTTON_UP_IO_isr_handler__when_input_is_close) {
    test_BUTTON_XXX_isr_handler("13", "12", "14", "0", BUTTON_UP_IO_CLOSE, false);
}

TEST_C(GpioTestsGroup, BUTTON_DOWN_IO_isr_handler__when_input_is_open) {
    test_BUTTON_XXX_isr_handler("12", "13", "14", "0", BUTTON_DOWN_IO_OPEN, true);
}

TEST_C(GpioTestsGroup, BUTTON_DOWN_IO_isr_handler__when_input_is_close) {
    test_BUTTON_XXX_isr_handler("12", "13", "14", "0", BUTTON_DOWN_IO_CLOSE, false);
}

TEST_C(GpioTestsGroup, BUTTON_RIGHT_IO_isr_handler__when_input_is_open) {
    test_BUTTON_XXX_isr_handler("0",
                                "13",
                                "12",
                                "14",
                                BUTTON_RIGHT_IO_OPEN | INPUT_1_IO_OPEN,
                                true);
}

TEST_C(GpioTestsGroup, BUTTON_RIGHT_IO_isr_handler__when_input_is_close) {
    test_BUTTON_XXX_isr_handler("0",
                                "13",
                                "12",
                                "14",
                                BUTTON_RIGHT_IO_CLOSE | INPUT_1_IO_CLOSE,
                                false);
}

TEST_C(GpioTestsGroup, BUTTON_SELECT_IO_isr_handler__when_input_is_open) {
    test_BUTTON_XXX_isr_handler("14", "13", "12", "0", BUTTON_SELECT_IO_OPEN, true);
}

TEST_C(GpioTestsGroup, BUTTON_SELECT_IO_isr_handler__when_input_is_close) {
    test_BUTTON_XXX_isr_handler("14", "13", "12", "0", BUTTON_SELECT_IO_CLOSE, false);
}

TEST_C(GpioTestsGroup, set_OUTPUT_0_to_active) {
    mock_scope_c("2")
        ->expectOneCall("gpio_set_level")
        ->withUnsignedIntParameters("level", GPIO_ACTIVE);
    mock_scope_c("15")->expectNoCall("gpio_set_level");

    set_digital_value(OUTPUT_0, true);
}

TEST_C(GpioTestsGroup, set_OUTPUT_0_to_passive) {
    mock_scope_c("2")
        ->expectOneCall("gpio_set_level")
        ->withUnsignedIntParameters("level", GPIO_PASSIVE);
    mock_scope_c("15")->expectNoCall("gpio_set_level");

    set_digital_value(OUTPUT_0, false);
}

TEST_C(GpioTestsGroup, set_OUTPUT_1_to_active) {
    mock_scope_c("2")->expectNoCall("gpio_set_level");
    mock_scope_c("15")
        ->expectOneCall("gpio_set_level")
        ->withUnsignedIntParameters("level", GPIO_ACTIVE);
    mock_scope_c("16")->expectOneCall("gpio_set_level")->ignoreOtherParameters();

    set_digital_value(OUTPUT_1, true);
}

TEST_C(GpioTestsGroup, set_OUTPUT_1_to_passive) {
    mock_scope_c("2")->expectNoCall("gpio_set_level");
    mock_scope_c("15")
        ->expectOneCall("gpio_set_level")
        ->withUnsignedIntParameters("level", GPIO_PASSIVE);
    mock_scope_c("16")->expectOneCall("gpio_set_level")->ignoreOtherParameters();

    set_digital_value(OUTPUT_1, false);
}