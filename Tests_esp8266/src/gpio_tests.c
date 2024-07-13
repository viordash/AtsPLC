#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport_c.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "gpio.c"

TEST_C(GpioTestsGroup, gpio_init__use_cleared_startup_value) {
    int event = 0;
    mock_c()->expectNCalls(2, "gpio_config")->ignoreOtherParameters();
    mock_c()->expectOneCall("adc_init")->ignoreOtherParameters();
    mock_c()->expectOneCall("xEventGroupCreate")->andReturnPointerValue(&event);
    mock_c()->expectOneCall("gpio_install_isr_service");
    mock_scope_c("0")->expectOneCall("gpio_isr_handler_add")->ignoreOtherParameters();
    mock_scope_c("12")->expectOneCall("gpio_isr_handler_add")->ignoreOtherParameters();
    mock_scope_c("13")->expectOneCall("gpio_isr_handler_add")->ignoreOtherParameters();
    mock_scope_c("14")->expectOneCall("gpio_isr_handler_add")->ignoreOtherParameters();

    mock_scope_c("2")->expectOneCall("gpio_set_level")->withUnsignedIntParameters("level", 0);
    mock_scope_c("15")->expectOneCall("gpio_set_level")->withUnsignedIntParameters("level", 0);

    gpio_init(0);
}

TEST_C(GpioTestsGroup, gpio_init__use_startup_for_out0) {
    int event = 0;
    mock_c()->expectNCalls(2, "gpio_config")->ignoreOtherParameters();
    mock_c()->expectOneCall("adc_init")->ignoreOtherParameters();
    mock_c()->expectOneCall("xEventGroupCreate")->andReturnPointerValue(&event);
    mock_c()->expectOneCall("gpio_install_isr_service");
    mock_scope_c("0")->expectOneCall("gpio_isr_handler_add")->ignoreOtherParameters();
    mock_scope_c("12")->expectOneCall("gpio_isr_handler_add")->ignoreOtherParameters();
    mock_scope_c("13")->expectOneCall("gpio_isr_handler_add")->ignoreOtherParameters();
    mock_scope_c("14")->expectOneCall("gpio_isr_handler_add")->ignoreOtherParameters();

    mock_scope_c("2")->expectOneCall("gpio_set_level")->withUnsignedIntParameters("level", 1);
    mock_scope_c("15")->expectOneCall("gpio_set_level")->withUnsignedIntParameters("level", 0);

    gpio_init(0x01);
}

TEST_C(GpioTestsGroup, gpio_init__use_startup_for_out1) {
    int event = 0;
    mock_c()->expectNCalls(2, "gpio_config")->ignoreOtherParameters();
    mock_c()->expectOneCall("adc_init")->ignoreOtherParameters();
    mock_c()->expectOneCall("xEventGroupCreate")->andReturnPointerValue(&event);
    mock_c()->expectOneCall("gpio_install_isr_service");
    mock_scope_c("0")->expectOneCall("gpio_isr_handler_add")->ignoreOtherParameters();
    mock_scope_c("12")->expectOneCall("gpio_isr_handler_add")->ignoreOtherParameters();
    mock_scope_c("13")->expectOneCall("gpio_isr_handler_add")->ignoreOtherParameters();
    mock_scope_c("14")->expectOneCall("gpio_isr_handler_add")->ignoreOtherParameters();

    mock_scope_c("2")->expectOneCall("gpio_set_level")->withUnsignedIntParameters("level", 0);
    mock_scope_c("15")->expectOneCall("gpio_set_level")->withUnsignedIntParameters("level", 1);

    gpio_init(0x02);
}

static void test_BUTTON_UP_IO_isr_handler(bool opened) {
    int event = 42;
    gpio_isr_t isr_handler = NULL;
    char buffer[32];
    sprintf(buffer, "0x%08X", opened ? BUTTON_UP_IO_OPEN : BUTTON_UP_IO_CLOSE);

    mock_c()->expectNCalls(2, "gpio_config")->ignoreOtherParameters();
    mock_c()->expectOneCall("adc_init")->ignoreOtherParameters();
    mock_c()->expectOneCall("xEventGroupCreate")->andReturnPointerValue(&event);
    mock_c()->expectOneCall("gpio_install_isr_service");
    mock_scope_c("0")->expectOneCall("gpio_isr_handler_add");

    mock_scope_c("12")->expectOneCall("gpio_isr_handler_add");
    mock_scope_c("12")->setPointerData("isr_handler", &isr_handler);
    mock_scope_c("12")->expectOneCall("gpio_get_level")->andReturnIntValue(opened ? 0 : 1);
    mock_scope_c(buffer)
        ->expectOneCall("xEventGroupSetBitsFromISR")
        ->withPointerParameters("xEventGroup", &event);

    mock_scope_c("13")->expectOneCall("gpio_isr_handler_add");
    mock_scope_c("14")->expectOneCall("gpio_isr_handler_add");

    mock_scope_c("2")->expectOneCall("gpio_set_level")->ignoreOtherParameters();
    mock_scope_c("15")->expectOneCall("gpio_set_level")->ignoreOtherParameters();

    gpio_init(0);
    isr_handler(NULL);
}

TEST_C(GpioTestsGroup, BUTTON_UP_IO_isr_handler__when_input_is_open) {
    test_BUTTON_UP_IO_isr_handler(true);
}

TEST_C(GpioTestsGroup, BUTTON_UP_IO_isr_handler__when_input_is_close) {
    test_BUTTON_UP_IO_isr_handler(false);
}