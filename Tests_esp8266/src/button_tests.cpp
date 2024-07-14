#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/button.cpp"
#include "main/gpio.h"

TEST_GROUP(ButtonTestsGroup){ //
                              TEST_SETUP(){}

                              TEST_TEARDOWN(){}
};

TEST(ButtonTestsGroup, handle_btDown) {
    TickType_t ticks = 0;
    mock()
        .expectOneCall("xTaskGetTickCount")
        .withOutputParameterReturning("ticks", &ticks, sizeof(ticks));

    button testable("test", BUTTON_UP_IO_CLOSE, BUTTON_UP_IO_OPEN, 0, 0);

    auto state = testable.handle(BUTTON_UP_IO_CLOSE);
    CHECK_EQUAL(button::state::btDown, state);

    ticks = 10;
    state = testable.handle(BUTTON_UP_IO_CLOSE);
    CHECK_EQUAL(button::state::btNone, state);
}

TEST(ButtonTestsGroup, handle_with_unfamiliar_bits_nothing_to_do) {
    mock().expectNoCall("xTaskGetTickCount");

    button testable("test", BUTTON_UP_IO_CLOSE, BUTTON_UP_IO_OPEN, 0, 0);

    auto state = testable.handle(BUTTON_DOWN_IO_CLOSE);
    CHECK_EQUAL(button::state::btNone, state);

    state = testable.handle(BUTTON_DOWN_IO_OPEN);
    CHECK_EQUAL(button::state::btNone, state);
}

TEST(ButtonTestsGroup, handle_normal_press) {
    TickType_t ticks = 0 / portTICK_PERIOD_MS;
    mock()
        .expectNCalls(2, "xTaskGetTickCount")
        .withOutputParameterReturning("ticks", &ticks, sizeof(ticks));

    button testable("test", BUTTON_UP_IO_CLOSE, BUTTON_UP_IO_OPEN, 0, 0);

    auto state = testable.handle(BUTTON_UP_IO_CLOSE);
    CHECK_EQUAL(button::state::btDown, state);

    ticks = 30 / portTICK_PERIOD_MS;
    state = testable.handle(BUTTON_UP_IO_OPEN);
    CHECK_EQUAL(button::state::btPressed, state);
}

TEST(ButtonTestsGroup, handle_press_when_ticks_overflowed) {
    TickType_t ticks = UINT32_MAX - 1;
    mock()
        .expectNCalls(2, "xTaskGetTickCount")
        .withOutputParameterReturning("ticks", &ticks, sizeof(ticks));

    button testable("test", BUTTON_UP_IO_CLOSE, BUTTON_UP_IO_OPEN, 0, 0);

    auto state = testable.handle(BUTTON_UP_IO_CLOSE);
    CHECK_EQUAL(button::state::btDown, state);

    ticks = 29 / portTICK_PERIOD_MS;
    state = testable.handle(BUTTON_UP_IO_OPEN);
    CHECK_EQUAL(button::state::btPressed, state);
}

TEST(ButtonTestsGroup, handle_longpress_when_ticks_overflowed) {
    TickType_t ticks = UINT32_MAX - 10;
    mock()
        .expectNCalls(2, "xTaskGetTickCount")
        .withOutputParameterReturning("ticks", &ticks, sizeof(ticks));

    button testable("test", BUTTON_UP_IO_CLOSE, BUTTON_UP_IO_OPEN, 0, 0);

    auto state = testable.handle(BUTTON_UP_IO_CLOSE);
    CHECK_EQUAL(button::state::btDown, state);

    ticks = 2900 / portTICK_PERIOD_MS;
    state = testable.handle(BUTTON_UP_IO_OPEN);
    CHECK_EQUAL(button::state::btLongPressed, state);
}

TEST(ButtonTestsGroup, handle_short_press) {
    volatile TickType_t ticks = 0;
    mock()
        .expectNCalls(2, "xTaskGetTickCount")
        .withOutputParameterReturning("ticks", (const void *)&ticks, sizeof(ticks));

    button testable("test", BUTTON_UP_IO_CLOSE, BUTTON_UP_IO_OPEN, 0, 0);

    auto state = testable.handle(BUTTON_UP_IO_CLOSE);
    CHECK_EQUAL(button::state::btDown, state);

    ticks = 2;
    state = testable.handle(BUTTON_UP_IO_OPEN);
    CHECK_EQUAL(button::state::btShortPressed, state);
}