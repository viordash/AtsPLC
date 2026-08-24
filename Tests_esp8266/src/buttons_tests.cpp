#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/buttons.cpp"

static ProcessWakeupService *wakeup_service;

TEST_GROUP(ButtonsTestsGroup){ //
                               TEST_SETUP(){ buttons = decltype(buttons){};
deffered_button_bits = 0;
wakeup_service = new ProcessWakeupService();
}

TEST_TEARDOWN() {
    buttons.buttons.clear();
    buttons.buttons.shrink_to_fit();
    delete wakeup_service;
}
}
;

static ButtonsPressType process_task_iteration(EventBits_t uxBits) {
    wakeup_service->RemoveExpired();
    return handle_buttons(uxBits, wakeup_service);
}

TEST(ButtonsTestsGroup, close_alone_does_not_report_press) {
    volatile uint64_t os_us = 10 * 1000;
    mock()
        .expectNCalls(3, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(BUTTON_UP_IO_CLOSE));
}

TEST(ButtonsTestsGroup, unfamiliar_bits_are_ignored) {
    volatile uint64_t os_us = 10 * 1000;
    mock()
        .expectNCalls(1, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(INPUT_1_IO_CLOSE));
}

TEST(ButtonsTestsGroup, press_is_reported_when_settle_time_expired) {
    volatile uint64_t os_us = 10 * 1000;
    mock()
        .expectNCalls(8, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(BUTTON_UP_IO_CLOSE));

    os_us = 110 * 1000;
    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(BUTTON_UP_IO_OPEN));

    os_us = 120 * 1000;
    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(0));

    os_us = 131 * 1000;
    CHECK_EQUAL(ButtonsPressType::UP_PRESSED, process_task_iteration(0));
}

TEST(ButtonsTestsGroup, repeated_open_extends_settle_time) {
    volatile uint64_t os_us = 10 * 1000;
    mock()
        .expectNCalls(10, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(BUTTON_UP_IO_CLOSE));

    os_us = 110 * 1000;
    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(BUTTON_UP_IO_OPEN));

    os_us = 118 * 1000;
    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(BUTTON_UP_IO_OPEN));

    os_us = 131 * 1000;
    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(0));

    os_us = 145 * 1000;
    CHECK_EQUAL(ButtonsPressType::UP_PRESSED, process_task_iteration(0));
}

TEST(ButtonsTestsGroup, close_after_open_cancels_deferred_press) {
    volatile uint64_t os_us = 10 * 1000;
    mock()
        .expectNCalls(12, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(BUTTON_UP_IO_CLOSE));

    os_us = 110 * 1000;
    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(BUTTON_UP_IO_OPEN));

    os_us = 115 * 1000;
    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(BUTTON_UP_IO_CLOSE));

    os_us = 160 * 1000;
    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(0));

    os_us = 200 * 1000;
    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(BUTTON_UP_IO_OPEN));

    os_us = 230 * 1000;
    CHECK_EQUAL(ButtonsPressType::UP_PRESSED, process_task_iteration(0));
}

TEST(ButtonsTestsGroup, close_and_open_coalesced_in_one_call_report_press) {
    volatile uint64_t os_us = 10 * 1000;
    mock()
        .expectNCalls(6, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED,
                process_task_iteration(BUTTON_UP_IO_CLOSE | BUTTON_UP_IO_OPEN));

    os_us = 40 * 1000;
    CHECK_EQUAL(ButtonsPressType::UP_PRESSED, process_task_iteration(0));
}

TEST(ButtonsTestsGroup, long_press_is_reported) {
    volatile uint64_t os_us = 10 * 1000;
    mock()
        .expectNCalls(7, "esp_timer_get_time")
        .withOutputParameterReturning("os_us", (const void *)&os_us, sizeof(os_us));

    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(BUTTON_UP_IO_CLOSE));

    os_us = 610 * 1000;
    CHECK_EQUAL(ButtonsPressType::NOTHING_PRESSED, process_task_iteration(BUTTON_UP_IO_OPEN));

    os_us = 640 * 1000;
    CHECK_EQUAL(ButtonsPressType::UP_LONG_PRESSED, process_task_iteration(0));
}
