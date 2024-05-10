#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport_c.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "gpio.c"

TEST_C(GpioTestsGroup, gpio_init__use_cleared_startup_value) {
    mock_c()->expectNCalls(2, "gpio_config")->ignoreOtherParameters();
    mock_c()->expectOneCall("adc_init")->ignoreOtherParameters();

    mock_c()
        ->expectOneCall("gpio_set_level_0")
        ->withUnsignedIntParameters("level", 0)
        ->ignoreOtherParameters();
    mock_c()
        ->expectOneCall("gpio_set_level_1")
        ->withUnsignedIntParameters("level", 0)
        ->ignoreOtherParameters();

    gpio_init(0);
}

TEST_C(GpioTestsGroup, gpio_init__use_startup_for_out0) {
    mock_c()->expectNCalls(2, "gpio_config")->ignoreOtherParameters();
    mock_c()->expectOneCall("adc_init")->ignoreOtherParameters();

    mock_c()
        ->expectOneCall("gpio_set_level_0")
        ->withUnsignedIntParameters("level", 1)
        ->ignoreOtherParameters();
    mock_c()
        ->expectOneCall("gpio_set_level_1")
        ->withUnsignedIntParameters("level", 0)
        ->ignoreOtherParameters();

    gpio_init(0x01);
}

TEST_C(GpioTestsGroup, gpio_init__use_startup_for_out1) {
    mock_c()->expectNCalls(2, "gpio_config")->ignoreOtherParameters();
    mock_c()->expectOneCall("adc_init")->ignoreOtherParameters();

    mock_c()
        ->expectOneCall("gpio_set_level_0")
        ->withUnsignedIntParameters("level", 0)
        ->ignoreOtherParameters();
    mock_c()
        ->expectOneCall("gpio_set_level_1")
        ->withUnsignedIntParameters("level", 1)
        ->ignoreOtherParameters();

    gpio_init(0x02);
}