#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/button.cpp"
#include "main/buttons.h"
#include "main/sys_gpio.h"

TEST_GROUP(ButtonTestsGroup){ //
                              TEST_SETUP(){}

                              TEST_TEARDOWN(){}
};

TEST(ButtonTestsGroup, handle_btDown) {
    uint64_t os_us = 0;
    mock()
        .expectOneCall("esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us, sizeof(os_us));

    button testable("test",
                    BUTTON_UP_IO_CLOSE,
                    BUTTON_UP_IO_OPEN,
                    TButtons::NOTHING_PRESSED,
                    TButtons::NOTHING_PRESSED);

    auto state = testable.handle(BUTTON_UP_IO_CLOSE);
    CHECK_EQUAL(button::state::btDown, state);

    os_us = 10 * 1000;
    state = testable.handle(BUTTON_UP_IO_CLOSE);
    CHECK_EQUAL(button::state::btNone, state);
}

TEST(ButtonTestsGroup, handle_with_unfamiliar_bits_nothing_to_do) {
    mock().expectNoCall("esp_timer_get_time");

    button testable("test",
                    BUTTON_UP_IO_CLOSE,
                    BUTTON_UP_IO_OPEN,
                    TButtons::NOTHING_PRESSED,
                    TButtons::NOTHING_PRESSED);

    auto state = testable.handle(BUTTON_DOWN_IO_CLOSE);
    CHECK_EQUAL(button::state::btNone, state);

    state = testable.handle(BUTTON_DOWN_IO_OPEN);
    CHECK_EQUAL(button::state::btNone, state);
}

TEST(ButtonTestsGroup, handle_normal_press) {
    uint64_t os_us = 0;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us, sizeof(os_us));

    button testable("test",
                    BUTTON_UP_IO_CLOSE,
                    BUTTON_UP_IO_OPEN,
                    TButtons::NOTHING_PRESSED,
                    TButtons::NOTHING_PRESSED);

    auto state = testable.handle(BUTTON_UP_IO_CLOSE);
    CHECK_EQUAL(button::state::btDown, state);

    os_us = 30 * 1000;
    state = testable.handle(BUTTON_UP_IO_OPEN);
    CHECK_EQUAL(button::state::btPressed, state);
}

TEST(ButtonTestsGroup, handle_press_when_os_us_overflowed) {
    uint64_t os_us = UINT64_MAX - 1 * 1000;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us, sizeof(os_us));

    button testable("test",
                    BUTTON_UP_IO_CLOSE,
                    BUTTON_UP_IO_OPEN,
                    TButtons::NOTHING_PRESSED,
                    TButtons::NOTHING_PRESSED);

    auto state = testable.handle(BUTTON_UP_IO_CLOSE);
    CHECK_EQUAL(button::state::btDown, state);

    os_us = 29 * 1000;
    state = testable.handle(BUTTON_UP_IO_OPEN);
    CHECK_EQUAL(button::state::btPressed, state);
}

TEST(ButtonTestsGroup, handle_longpress_when_os_us_overflowed) {
    uint64_t os_us = UINT64_MAX - 100 * 1000;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", &os_us, sizeof(os_us));

    button testable("test",
                    BUTTON_UP_IO_CLOSE,
                    BUTTON_UP_IO_OPEN,
                    TButtons::NOTHING_PRESSED,
                    TButtons::NOTHING_PRESSED);

    auto state = testable.handle(BUTTON_UP_IO_CLOSE);
    CHECK_EQUAL(button::state::btDown, state);

    os_us = 2900 * 1000;
    state = testable.handle(BUTTON_UP_IO_OPEN);
    CHECK_EQUAL(button::state::btLongPressed, state);
}

TEST(ButtonTestsGroup, handle_short_press) {
    volatile uint64_t os_us = 0;
    mock()
        .expectNCalls(2, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    button testable("test",
                    BUTTON_UP_IO_CLOSE,
                    BUTTON_UP_IO_OPEN,
                    TButtons::NOTHING_PRESSED,
                    TButtons::NOTHING_PRESSED);

    auto state = testable.handle(BUTTON_UP_IO_CLOSE);
    CHECK_EQUAL(button::state::btDown, state);

    os_us = 2 * 1000;
    state = testable.handle(BUTTON_UP_IO_OPEN);
    CHECK_EQUAL(button::state::btShortPressed, state);
}