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

    mock_scope_c("2")
        ->expectOneCall("gpio_set_level")
        ->withUnsignedIntParameters("level", 0)
        ->ignoreOtherParameters();
    mock_scope_c("15")
        ->expectOneCall("gpio_set_level")
        ->withUnsignedIntParameters("level", 0)
        ->ignoreOtherParameters();

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

    mock_scope_c("2")
        ->expectOneCall("gpio_set_level")
        ->withUnsignedIntParameters("level", 1)
        ->ignoreOtherParameters();
    mock_scope_c("15")
        ->expectOneCall("gpio_set_level")
        ->withUnsignedIntParameters("level", 0)
        ->ignoreOtherParameters();

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

    mock_scope_c("2")
        ->expectOneCall("gpio_set_level")
        ->withUnsignedIntParameters("level", 0)
        ->ignoreOtherParameters();
    mock_scope_c("15")
        ->expectOneCall("gpio_set_level")
        ->withUnsignedIntParameters("level", 1)
        ->ignoreOtherParameters();

    gpio_init(0x02);
}